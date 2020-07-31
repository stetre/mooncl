#!/usr/bin/env lua
-- Matrix-vector multiplication

local cl = require('mooncl')

-- Identify a platform, access a device and create a CL context:
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})

-- Create a command queue for the device:
local queue = cl.create_command_queue(context, device)

-- Read the kernel program's source code, and create the program object:
local file = assert(io.open("matvec.cl", 'r'))
local source = file:read('a')
file:close()
local program = cl.create_program_with_source(context, source)

-- Build the program:
cl.build_program(program, {device}, nil)

-- Get the matvec_mult kernel from the program:
local kernel = cl.create_kernel(program, "matvec_mult")

-- Prepare the data to be processed by the kernel:
local mat, vec = {}, {}
for i = 1, 16 do mat[i] = (i-1)*2.0*math.pi end
for i = 1, 4 do vec[i] = (i-1)*3.0 end

-- Create the buffers to hold input and output data:
local mat_mem = cl.malloc('float', mat) -- same as cl.malloc(cl.pack('float', mat))
local vec_mem = cl.malloc('float', vec)
local mat_buf = cl.create_buffer(context,
         cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, mat_mem:size(), mat_mem:ptr())
local vec_buf = cl.create_buffer(context,
         cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, vec_mem:size(), vec_mem:ptr())
local result_size =  cl.sizeof('float')*4
local res_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, result_size)

-- Set the kernel arguments and enqueue the kernel execution command: 
cl.set_kernel_arg(kernel, 0, mat_buf)
cl.set_kernel_arg(kernel, 1, vec_buf)
cl.set_kernel_arg(kernel, 2, res_buf)
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {4}, nil)

-- Read back the result: 
local mem = cl.malloc(result_size)
cl.enqueue_read_buffer(queue, res_buf, true, 0, mem:size(), mem:ptr()) 
local result = mem:read(0, result_size, 'float')
--[[ same as:
local result_data = mem:read(0, result_size)   -- read the data, returns a binary string
local result = cl.unpack('float', result_data) -- interpret the binary as a sequence of floats
--]]

-- Compute the correct result to check against the result from the kernel.
-- Note that this is computed using double precision, while the kernel used single
-- precision (float), so we cannot expect them to be exactly equal.
local correct = {0, 0, 0, 0}
for i = 1, 4 do
   correct[1] = correct[1] + mat[i]*vec[i]
   correct[2] = correct[2] + mat[i+4]*vec[i]
   correct[3] = correct[3] + mat[i+8]*vec[i]
   correct[4] = correct[4] + mat[i+12]*vec[i]
end

print('result', table.unpack(result))
print('correct', table.unpack(correct))

-- Test the result: 
local ok = true
local epsilon = 1e-3
for i=1,4 do 
   if math.abs(result[i] - correct[i]) > epsilon then ok = false break end 
end
print("Matrix-vector multiplication " .. (ok and "successful" or "unsuccessful"))
   
-- Deallocate resources 
cl.release_context(context)

