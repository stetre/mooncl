#!/usr/bin/env lua
-- Retaining and releasing an OpenCL context
local cl = require('mooncl')

cl.trace_objects(true) -- uncomment to trace creation/deletion of objects

-- Access the first platform:
local platform = cl.get_platform_ids()[1]

-- Access a device, preferably a GPU, otherwise a CPU:
local ok, devices = pcall(cl.get_device_ids, platform, cl.DEVICE_TYPE_GPU)
if not ok then devices = cl.get_device_ids(platform, cl.DEVICE_TYPE_CPU) end
local device = devices[1]

-- Create a context:
local context = cl.create_context(platform, {device})

local ref_count = cl.get_context_info(context, 'reference count')
print("Initial reference count: " .. ref_count)

cl.retain_context(context)
ref_count = cl.get_context_info(context, 'reference count')
print("Reference count: " .. ref_count)

cl.release_context(context) -- this should just decrease the reference count
ref_count = cl.get_context_info(context, 'reference count')
print("Reference count: " .. ref_count)

cl.release_context(context) -- this should delete the context
print("Exiting")

