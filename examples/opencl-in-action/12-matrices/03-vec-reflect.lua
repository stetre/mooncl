#!/usr/bin/env lua
-- Householder reflection

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, "vec_reflect.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "vec_reflect")

-- Set arguments and enqueue kernel
local x = {1.0, 2.0, 3.0, 4.0}
local u = {0.0, 5.0, 0.0, 0.0}
local buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 4*cl.sizeof('float'))
cl.set_kernel_arg(kernel, 0, 'float', x)
cl.set_kernel_arg(kernel, 1, 'float', u)
cl.set_kernel_arg(kernel, 2, buf)
cl.enqueue_task(queue, kernel) 

-- Read back and print the result 
local mem = cl.malloc(4*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
local reflect = mem:read(0, nil, 'float')
io.write(string.format("Result: %f %f %f %f\n", table.unpack(reflect)))

-- For comparison, compute the result also in Lua using MoonGLMATH (if available)
local ok, glmath = pcall(require, "moonglmath")
if ok then
    local vec4 = glmath.vec4
    local x = vec4(table.unpack(x))
    local u = vec4(table.unpack(x))
    local r = x - 2*u*(u*x)/u:norm()^2
    io.write(string.format("Result (in Lua): %f %f %f %f\n", table.unpack(r)))
end

