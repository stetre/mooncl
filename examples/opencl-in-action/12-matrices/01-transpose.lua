#!/usr/bin/env lua
-- Matrix transposition

local cl = require('mooncl')
-- cl.trace_objects(true)

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel
local program = cl.create_program_with_sourcefile(context, "transpose.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "transpose")

-- Input data (a NxN matrix)
local N = 64 -- matrix dimension (N x N)
local matrix = {}
for i=1,N do
    matrix[i] = {}
    for j=1,N do matrix[i][j] = (i-1)*N + j-1 end
end

local mem = cl.malloc('float', matrix)
local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Determine execution parameters
local global_size = (N/4 * (N/4 + 1))/2
local local_mem_size = cl.get_device_info(device, 'local mem size') -32
-- (for some reason I cannot allocate local_mem_size bytes on my GPU, but I can
--  allocate 32 bytes less than that...)

-- Set arguments and enqueue kernel for execution
local size = N/4
cl.set_kernel_arg(kernel, 0, buf)
cl.set_kernel_arg(kernel, 1, local_mem_size)
cl.set_kernel_arg(kernel, 2, 'uint', size)
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {global_size})

-- Read back the result and check it
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
local transpose = {}
local rowsize = N*cl.sizeof('float')
for i=1,N do transpose[i] = mem:read((i-1)*rowsize, rowsize, 'float') end

local ok = true
for i=1,N do
    for j=1,N do 
        if transpose[i][j] ~= matrix[j][i] then ok=false break end
    end
end
print("Transpose check "..(ok and "succeeded." or "failed."))

