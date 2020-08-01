#!/usr/bin/env lua
-- Create a buffer and a sub-buffer within it, and query some info about them.

local cl = require('mooncl')

local function create_device()
-- Finds a GPU or CPU associated with the first available platform, and
-- returns both the device and the platform.
   local platform = cl.get_platform_ids()[1]
   local ok, devices = pcall(cl.get_device_ids, platform, cl.DEVICE_TYPE_GPU)
   if not ok then devices = cl.get_device_ids(platform, cl.DEVICE_TYPE_CPU) end
   return devices[1], platform
end

-- Create device and context 
local device, platform = create_device()
local context = cl.create_context(platform, {device})

-- Allocate host memory to hold 100 floating-point values:
local datasize = 100*cl.sizeof('float')
local main_data = cl.malloc(datasize)
 
-- Create a CL buffer to hold the values.
local main_buffer = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, main_data:size(), main_data:ptr())
 
-- Create a sub-buffer 
local origin = 0x100 -- offset within the buffer
local size = 20*cl.sizeof('float')
local sub_buffer = cl.create_buffer_region(main_buffer, cl.MEM_READ_ONLY, origin, size)
 
--[[
local flags = cl.get_mem_object_info(main_buffer, 'flags')
print("'"..table.concat({cl.memflags(flags)}, "', ").."'")
--]]

-- Obtain size information about the buffers 
local main_buffer_size = cl.get_mem_object_info(main_buffer, 'size')
local sub_buffer_size = cl.get_mem_object_info(sub_buffer, 'size')
print("Main buffer size: ", main_buffer_size)
print("Sub-buffer size: ", sub_buffer_size)

-- Obtain the host pointers 
local main_buffer_ptr = cl.get_mem_object_info(main_buffer, 'host ptr')
local sub_buffer_ptr = cl.get_mem_object_info(sub_buffer, 'host ptr')
print("Main buffer memory address: ", main_buffer_ptr)
print("Sub-buffer memory address: ", sub_buffer_ptr)
-- Print the address of the main data 
print("Main array address: ", main_data:ptr()) 

-- Deallocate resources 
cl.release_buffer(main_buffer) -- this causes the deletion of the buffer and of any sub buffer
-- cl.release_buffer(sub_buffer) -- this would cause an error because sub_buffer is already released
cl.release_context(context)

