#!/usr/bin/env lua
-- Lua/MoonCL implementation of the Hello World example from the
-- "Introduction to OpenCL" course by David Black-Schaffer"
-- https://www.youtube.com/playlist?list=PLiwt1iVUib9s6vyEqdpcgAq7NBRlp9mAY

local cl = require('mooncl')
 
-- Setup OpenCL
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_GPU)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Compile the kernel
local source = [[
kernel void calc_sin(global float *data)
   {
   int id = get_global_id(0);
   data[id] = sin(data[id]);
   }
]]

local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "calc_sin")

-- Prepare the input data
local LENGTH = 4096
local data = {}
for i=1,LENGTH do data[i] = i-1 end

-- Create a hostmem object to hold the data in host memory
local mem = cl.malloc('float', data)

-- Create a buffer to hold the data on the GPU
local buffer = cl.create_buffer(context, cl.MEM_READ_WRITE, mem:size())

-- Write the data to the buffer (ie, transfer it from the host to the GPU)
cl.enqueue_write_buffer(queue, buffer, true, 0, mem:size(), mem:ptr())

-- Execute the kernel
cl.set_kernel_arg(kernel, 0, buffer)
cl.enqueue_ndrange_kernel(queue, kernel, 1, {0}, {LENGTH}, nil)

-- Read back the result 
cl.enqueue_read_buffer(queue, buffer, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'float')

-- Check the result
local epsilon = 2^-23
for i = 1, LENGTH do
   assert(math.abs(result[i] - math.sin(i-1)) < epsilon, "Result check failed")
end

print("Success")

-- Deallocate resources 
cl.release_context(context)

