#!/usr/bin/env lua
-- Matrix multiplication

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)
-- Build program and create kernels 
local program = cl.create_program_with_sourcefile(context, "matrix_mult.cl")
cl.build_program(program, {device})
local transpose_kernel = cl.create_kernel(program, "transpose")
local mult_kernel = cl.create_kernel(program, "matrix_mult")

-- Create input/output matrices and corresponding buffers
math.randomseed(os.time())
local function new_mat(n, val)
-- Returns an nxn matrix with all elements initialized to val, or 
-- with random elements (if val==nil)
    local m = {}
    for i=1, n do 
        m[i] = {}
        for j=1, n do m[i][j] = (val and val or math.random()) end
    end
    return m
end

local N = 128 -- we're working with NxN matrices
local A = new_mat(N)
local B = new_mat(N)
local C = new_mat(N, 0) -- A*B for result check
local t = cl.now()
for i = 1, N do
    for j = 1, N do
        for k = 1, N do
        C[i][j] = C[i][j] + A[i][k]*B[k][j]
        end
    end
end
t = cl.since(t)
printf("Lua execution time:    %.6f [s]\n", t)

local t = cl.now()
local a_mem = cl.malloc('float', A)
local b_mem = cl.malloc('float', B)
local c_mem = cl.malloc(a_mem:size())
a_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, a_mem:size(), a_mem:ptr())
b_buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, b_mem:size(), b_mem:ptr()) 
c_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, c_mem:size())

-- Determine device memory parameters
local global_size = (N/4 * (N/4 + 1))/2
local local_mem_size = cl.get_device_info(device, 'local mem size')-32
-- (for some reason I cannot allocate local_mem_size bytes on my GPU, but I can
--  allocate 32 bytes less than that...)

-- Execute transpose kernel
local dim = N/4
cl.set_kernel_arg(transpose_kernel, 0, b_buf)
cl.set_kernel_arg(transpose_kernel, 1, local_mem_size-32)
cl.set_kernel_arg(transpose_kernel, 2, 'uint', dim)
cl.enqueue_ndrange_kernel(queue, transpose_kernel, 1, nil, {global_size})

-- Execute multiplication kernel
global_size = N
cl.set_kernel_arg(mult_kernel, 0, a_buf)
cl.set_kernel_arg(mult_kernel, 1, b_buf)
cl.set_kernel_arg(mult_kernel, 2, c_buf)
cl.enqueue_ndrange_kernel(queue, mult_kernel, 1, nil, {global_size})

-- Read back the result and unpack it
cl.enqueue_read_buffer(queue, c_buf, true, 0, c_mem:size(), c_mem:ptr())
local result = {}
local rowsize = N*cl.sizeof('float')
for i=1,N do result[i] = c_mem:read((i-1)*rowsize, rowsize, 'float') end
t = cl.since(t)
printf("MoonCL execution time: %.6f [s]\n", t)

-- Check the result
local ok = true
for i=1, N do
    for j=1, N do
    if math.abs(result[i][j] - C[i][j]) > 0.01 then ok = false; break end
    end
end
print("Multiplication check " .. (ok and "succeeded." or "failed."))

