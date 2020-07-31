#!/usr/bin/env lua
-- Create a program with multiple kernels and query for their names.

local cl = require('mooncl')
--cl.trace_objects(true)

local source = [[
kernel void add(global float *a, global float *b, global float *c) { *c = *a + *b; }
kernel void sub(global float *a, global float *b, global float *c) { *c = *a - *b; }
kernel void mult(global float *a, global float *b, global float *c) { *c = *a * *b; }
kernel void div(global float *a, global float *b, global float *c) { *c = *a / *b; }
]]

-- Access the first device of the first platform, and create a context:
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})

-- Create a program from the above source, and build it:
local program = cl.create_program_with_source(context, source)
cl.build_program(program)

-- Create a kernel for each function:
local kernels = cl.create_kernels_in_program(program) 

-- Search for the named kernel:
local found
for i = 1, #kernels do
   local name = cl.get_kernel_info(kernels[i], 'function name')     
   print("kernel #"..i..": "..name)
   if name == "mult" then found = i end
end

if found then
   print("found 'mult' kernel at index " .. found)
else
   print("'mult' kernel not found " .. found)
end

-- Cleanup is unneeded, since we're exiting
