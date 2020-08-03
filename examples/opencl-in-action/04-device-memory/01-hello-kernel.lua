#!/usr/bin/env lua
-- Execution of a basic kernel

local cl = require('mooncl')

-- cl.trace_objects(true)

local kernelsource = [[
kernel void hello_kernel(global char16 *msg) {
   *msg = (char16)('H', 'e', 'l', 'l', 'o', ' ',
      'k', 'e', 'r', 'n', 'e', 'l', '!', '!', '!', '\0');
}]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "hello_kernel")

-- Create a buffer to hold the output data 
local size = 16*cl.sizeof('char')
local buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

-- Enqueue kernel for execution, passing to it the buffer as argument
cl.set_kernel_arg(kernel, 0, buffer)
cl.enqueue_task(queue, kernel)

-- Read the buffer to host memory, and print the result 
local mem = cl.malloc(size)
cl.enqueue_read_buffer(queue, buffer, true, 0, mem:size(), mem:ptr())
print("Kernel output: "..mem:read())

-- Deallocate resources.
-- In this case we could avoid releasing resources, because we're exiting and
-- thus cleanup is performed automatically. We do it anyway as an example of 
-- manual cleanup.
cl.release_context(context) -- this also releases anything associated with the
                            -- context (queue, kernel, program, and buffer)
mem:free()
cl.release_device(device)
platform:delete()
print("Done")
