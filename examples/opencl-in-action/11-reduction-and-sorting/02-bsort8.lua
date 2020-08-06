#!/usr/bin/env lua
-- An eight-element bitonic sort

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, "bsort8.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "bsort8")

local ascending, descending = 0, -1
local dir = ascending
if arg[1] == 'descending' then dir=descending end

-- Prepare input data 
local data = { 3.0, 5.0, 4.0, 6.0, 0.0, 7.0, 2.0, 1.0 }
local mem = cl.malloc('float', data)
local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())
printf("Input:  %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\n",
       data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8])

-- Set arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, buf)
cl.set_kernel_arg(kernel, 1, 'int', dir)
cl.enqueue_task(queue, kernel) 

-- Read, unpack, and print the result 
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
local data = mem:read(0, nil, 'float')
printf("Output:  %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f %3.1f\n",
       data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8])

-- Check the result
local ok = true
if dir == ascending then 
   for i=2, 8 do if data[i] < data[i-1] then ok=false; break end end
elseif dir == descending then
   for i=2, 8 do if data[i] > data[i-1] then ok=false; break end end
end
print("Bitonic sort "..(ok and "succeeded." or "failed."))

