#!/usr/bin/env lua
-- Map a buffer to host memory and read from it.

local cl = require('mooncl')
-- cl.trace_objects(true)

local kernelsource = [[__kernel void blank(__global float *a, __global float *b) { }]]

local function display_matrix(preamble, matrix, rows, columns)
   if preamble then print(preamble) end
   for i=1,rows do
      for j=1,columns do io.write(string.format("%6.1f", matrix[1+(j-1)+(i-1)*10])) end
      io.write("\n")
   end
end

-- Get a device, create a context and a command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build the program and create the kernel 
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program)
local kernel = cl.create_kernel(program, 'blank')

-- Initialize input arrays
local data_one, data_two = {}, {}
for i=1,100 do
   data_one[i] = i-1
   data_two[i] = -(i-1)
end
local mem_one = cl.malloc('float', data_one)
local mem_two = cl.malloc('float', data_two)
local size = mem_one:size() -- = mem_two:size()

-- Create buffers, and initialize them with the arrays
local buffer_one = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, size, mem_one:ptr())
local buffer_two = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, size, mem_two:ptr())

-- Set buffers as arguments and enqueue the do-nothing kernel for execution
cl.set_kernel_arg(kernel, 0, buffer_one)
cl.set_kernel_arg(kernel, 1, buffer_one)
cl.enqueue_task(queue, kernel)

-- Enqueue a command to copy the first half of buffer_one to buffer_two:
cl.enqueue_copy_buffer(queue, buffer_one, buffer_two, 0, 0, size/2)

-- Enqueue a command to map buffer_two to host memory. This returns a pointer (userdata)
-- to a host-accessible memory-mapped region:
local mapped_ptr = cl.enqueue_map_buffer(queue, buffer_two, true, cl.MAP_READ, 0, size)
-- Encapsulate the mapped memory in an hostmem object, so we can read its contents to a
-- table using the hostmem objects methods:
local mapped_mem = cl.hostmem(size, mapped_ptr)
local output = mapped_mem:read(0, nil, 'float')

-- Display updated buffer 
display_matrix("data_one:", data_one, 10, 10)
display_matrix("data_two:", data_two, 10, 10)
display_matrix("output:", output, 10, 10)
print(mem_one:ptr())
print(mem_two:ptr())
print(mapped_ptr)

-- Deallocate resources 
cl.release_context(context)

