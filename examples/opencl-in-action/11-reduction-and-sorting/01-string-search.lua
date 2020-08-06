#!/usr/bin/env lua
-- Implementing MapReduce in OpenCL

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, "string_search.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "string_search")

-- Determine the parameters for the work space
local max_units = cl.get_device_info(device, 'max compute units')
local local_size = cl.get_device_info(device, 'max work group size')
local global_size = max_units * local_size
local global_offset = 0

-- Read text file and place content into buffer 
local file = assert(io.open("kafka.txt", 'r'))
local text = file:read('a')
file:close()
-- Encapsulate the text in a hostmem object, so to have access to a pointer to it
local mem = cl.malloc(text)
-- Create a buffer to hold the text characters
local text_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())
-- Compute the size of the data chunk processed by a work-item
local chars_per_item = math.floor(mem:size()/global_size) + 1
mem:free() -- we don't need this any more

-- Create a buffer for the global result
local result_size = 4*cl.sizeof('int')
local result_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, result_size)

-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, nil, "thatwithhavefrom")
cl.set_kernel_arg(kernel, 1, text_buf)
cl.set_kernel_arg(kernel, 2, 'int', chars_per_item)
cl.set_kernel_arg(kernel, 3, result_size) -- allocates local memory of the given size
cl.set_kernel_arg(kernel, 4, result_buf)
cl.enqueue_ndrange_kernel(queue, kernel, 1, {global_offset}, {global_size}, {local_size}) 

-- Read, unpack, and print the result 
local mem = cl.malloc(result_size)
cl.enqueue_read_buffer(queue, result_buf, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'int')

printf("Number of occurrences of 'that': %d\n", result[1])
printf("Number of occurrences of 'with': %d\n", result[2])
printf("Number of occurrences of 'have': %d\n", result[3])
printf("Number of occurrences of 'from': %d\n", result[4])

