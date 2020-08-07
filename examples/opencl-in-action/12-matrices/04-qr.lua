#!/usr/bin/env lua
-- QR decomposition with Householder algorithm

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, "qr.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "qr")

local N = 32 -- we're working with NxN matrices

-- Initialize matrix A
local A = {}
math.randomseed(os.time())
for i= 1,N do
    A[i] = {}
    for j = 1,N do A[i][j] = math.random() end
end
local mem = cl.malloc('float', A)

-- Create buffers 
local a_buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())
local q_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, mem:size())
local p_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, mem:size())
local prod_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, mem:size())

-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, N*cl.sizeof('float'))
cl.set_kernel_arg(kernel, 1, a_buf)
cl.set_kernel_arg(kernel, 2, q_buf)
cl.set_kernel_arg(kernel, 3, p_buf)
cl.set_kernel_arg(kernel, 4, prod_buf)
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {N}, {N})

-- Read back the results and unpack them
local rowsize = N*cl.sizeof('float')
cl.enqueue_read_buffer(queue, q_buf, true, 0, mem:size(), mem:ptr())
local Q={}
for i=1,N do Q[i] = mem:read((i-1)*rowsize, rowsize, 'float') end
cl.enqueue_read_buffer(queue, a_buf, true, 0, mem:size(), mem:ptr())
local R={}
for i=1,N do R[i] = mem:read((i-1)*rowsize, rowsize, 'float') end

-- Compute QR = Q*R and check if it is equal to A 
local QR = {}
for i= 1, N do
    QR[i] = {}
    for j = 1,N do
        QR[i][j] = 0
        for k = 1,N do QR[i][j] = QR[i][j] + Q[i][k]*R[k][j] end
    end
end
local ok = true
for i=1, N do
    for j=1, N do
    if math.abs(A[i][j] - QR[i][j]) > 0.01 then ok = false break end
    end
end

print("QR decomposition check ".. (ok and "succeeded." or "failed."))

