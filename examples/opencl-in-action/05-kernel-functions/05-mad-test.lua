#!/usr/bin/env lua
-- Multiply-and-add large numbers

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void mad_test(global uint *result)
   {
   uint a = 0x123456;
   uint b = 0x112233;
   uint c = 0x111111;
   result[0] = mad24(a, b, c);
   result[1] = mad_hi(a, b, c);
   }
]]
 
-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "mad_test")

-- Create a write-only buffer to hold the output data
local result_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 2*cl.sizeof('uint'))
         
-- Set kernel argument and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, result_buffer)
cl.enqueue_task(queue, kernel)

-- Read back and print the result 
local mem = cl.malloc(2*cl.sizeof('uint'))
cl.enqueue_read_buffer(queue, result_buffer, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'uint')
   
io.write(string.format("Result of multiply-and-add: 0x%X%X\n", result[2], result[1]))

