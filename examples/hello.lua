#!/usr/bin/env lua
-- MoonCL example: hello.lua
cl = require("mooncl")

-- Select platform and device, create context and command_queue
platform = cl.get_platform_ids()[1]
device = cl.get_device_ids(platform, cl.DEVICE_TYPE_GPU)[1]
context = cl.create_context(platform, {device})
queue = cl.create_command_queue(context, device)

-- Create program and build it
program = cl.create_program_with_source(context,[[
   kernel void myadd(uint a, uint b, global uint *result) {
      *result = a + b;
   }
]]) 

cl.build_program(program, {device})
 
-- Create kernel 
kernel = cl.create_kernel(program, "myadd")

-- Create a buffer to hold the result
result_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, cl.sizeof('uint'))
         
-- Set kernel arguments 
a, b = 20, 30
cl.set_kernel_arg(kernel, 0, 'uint', a)
cl.set_kernel_arg(kernel, 1, 'uint', b)
cl.set_kernel_arg(kernel, 2, result_buffer)

-- Enqueue kernel for execution
cl.enqueue_task(queue, kernel)

-- Read and print the result 
mem = cl.malloc(cl.sizeof('uint'))
cl.enqueue_read_buffer(queue, result_buffer, true, 0, mem:size(), mem:ptr())
result = mem:read(0, nil, 'uint')
   
-- Print result, and check it
print("".. a .. " + " .. b .. " = " .. result[1])
assert(result[1] == a + b)

-- Deallocate resources 
cl.release_context(context)


