#!/usr/bin/env lua
-- Displaying the bytes of a vector

local cl = require('mooncl')

if arg[1] == 'trace' then cl.trace_objects(true) end

local kernelsource = [[
kernel void vector_bytes(global uchar16 *test)
    {
    /* Initialize a vector of four integers */
    uint4 vec = {0x00010203, 0x04050607, 0x08090A0B, 0x0C0D0E0F}; 

    /* Convert the uint4 to a uchar16 byte-by-byte */
    uchar *p = &vec;
    *test = (uchar16)(*p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), 
            *(p+6), *(p+7), *(p+8), *(p+9), *(p+10), *(p+11), *(p+12), 
            *(p+13), *(p+14), *(p+15));
    }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build a program and create a kernel 
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program, {device}, options)
local kernel = cl.create_kernel(program, "vector_bytes")

-- Create a write-only buffer to hold the output data 
local test_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 16*cl.sizeof('uchar'))

-- Set kernel argument and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, test_buffer)
cl.enqueue_task(queue, kernel)

-- Read back and print the result 
local mem = cl.malloc(16*cl.sizeof('uchar'))
cl.enqueue_read_buffer(queue, test_buffer, true, 0, mem:size(), mem:ptr())
local test = mem:read(0, nil, 'uchar')

for i=1, #test do io.write(string.format("0x%X, ", test[i])) end
io.write('\n')

