#!/usr/bin/env lua
-- An eight-element radix sort

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, "radix_sort8.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "radix_sort8")

-- Input data
local N = 8
local data = {}
math.randomseed(os.time())
for i = 1, N do data[i] = i-1 end
for i = 1, N-1 do 
   local j = math.random(i, N)
   data[i], data[j] = data[j], data[i]
end
local mem = cl.malloc('ushort', data)
local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Set argument and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, buf)
cl.enqueue_task(queue, kernel) 

-- Read and print the result 
print("Input:  "..table.concat(data, ', '))
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
data = mem:read(0, nil, 'ushort')
print("Output: "..table.concat(data, ', '))

-- Check result
local ok = true
for i=1, N do
   if data[i] ~= i-1 then ok = false; break end
end
print("The radix sort ".. (ok and "succeeded." or "failed."))
