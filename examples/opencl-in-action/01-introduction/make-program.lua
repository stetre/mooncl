#!/usr/bin/env lua
-- This example shows three alternative ways to create and build a program
-- given an OpenCL C source file.

local cl = require('mooncl')
cl.trace_objects(true) -- trace the creation and deletion of objects

-- Identify a platform, access a device and create a CL context:
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})

-- Read the kernel program's source code, create the program object, and build it

-- Alt 1:
local file = assert(io.open("matvec.cl", 'r'))
local source = file:read('a')
file:close()
local program1 = cl.create_program_with_source(context, source)
cl.build_program(program1, {device}, nil)

-- Alt 2:
local program2 = cl.create_program_with_sourcefile(context, "matvec.cl")
cl.build_program(program2, {device}, nil)

-- Alt 3:
local program3 = cl.make_program_with_sourcefile(context, "matvec.cl")

-- Deallocate resources 
-- cl.release_context(context)

