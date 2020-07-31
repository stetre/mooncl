#!/usr/bin/env lua
-- Create a queue and enqueue a command for execution.

local cl = require('mooncl')

-- Access the first device of the first platform:
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]

-- Create a context, and a command queue for the device.
-- Note that a context may comprise multiple devices, but command queues are device-specific
-- (we may create multiple queues for the same device, though).
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Create a program and build it
local source = "kernel void blank() { }" -- just a kernel that does nothing
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})

-- Get the kernel
local kernel = cl.create_kernel(program, 'blank')
   
-- Enqueue the kernel execution command. 
print("Executing")
cl.enqueue_task(queue, kernel)
cl.finish(queue)
print("Exiting")

