#!/usr/bin/env lua
-- Write-to/read-from buffers.

local cl = require('mooncl')
--cl.trace_objects(true)

if arg[1] == 'trace' then cl.trace_objects(true) end

local function display_matrix(matrix, rows, columns)
   for i=1,rows do
      for j=1,columns do io.write(string.format("%6.1f", matrix[1+(j-1)+(i-1)*10])) end
      io.write("\n")
   end
end

-- Get a device, then create a context and a command queue for the device
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build the program and create the kernel 
local kernelsource = "kernel void blank(__global float *a) { }"
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program)
local kernel = cl.create_kernel(program, 'blank')

local floatsz = cl.sizeof('float')

-- Create and initialize the input matrix data
local input_matrix = {}
for i=1, 80 do input_matrix[i] = i-1 end
-- Allocate host memory and initialize it with the matrix data
local mem = cl.malloc('float', input_matrix)
-- Create a buffer to hold the input matrix
local buffer = cl.create_buffer(context, cl.MEM_READ_WRITE, mem:size())

-- Set buffer as 0-th argument to the kernel and enqueue kernel for execution
-- (this kernel does nothing, by the way)
cl.set_kernel_arg(kernel, 0, buffer)
cl.enqueue_task(queue, kernel)

-- Enqueue a command to write the input matrix to the buffer 
cl.enqueue_write_buffer(queue, buffer, true, 0, mem:size(), mem:ptr()) 

-- Clear the host memoryarea and enqueue a command to read into it
-- a rectangle of data from the input buffer
mem:clear(0, mem:size())
local buffer_origin = { 5*floatsz, 3, 0 } -- origin of the rectangle where to copy from
local host_origin = { 1*floatsz, 1, 0 }   -- origin of the rectangle where to copy to
local region = { 4*floatsz, 4, 1 }        -- dimensions of the rectangle to copy
local row_pitch = 10*floatsz
cl.enqueue_read_buffer_rect(queue, buffer, true, buffer_origin, host_origin, region, 
      row_pitch, 0, row_pitch, 0, mem:ptr())

-- Unpack the contents of the host memory into a Lua table:
--local output_matrix = cl.unpack('float', mem:read())
local output_matrix = mem:read(0, nil, 'float') -- read and unpack (more efficient)

-- Print the input and output matrices, for comparison. The output matrix should be
-- composed of all zeros except for the rectangle that was copied from the input: 
print("Input matrix:")
display_matrix(input_matrix, 8, 10)
print("Output matrix:")
display_matrix(output_matrix, 8, 10)

-- Deallocate resources 
cl.release_context(context) -- deallocates also all context-related resources

