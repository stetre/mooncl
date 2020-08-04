#!/usr/bin/env lua
-- Profiling data transfer

local cl = require('mooncl')

local NUM_BYTES = 2^17 --131072
local NUM_ITERATIONS = 2000

-- cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

local source = [[
kernel void init(global char16 *c, int num) // num= # of char16 vectors
   { for(int i=0; i<num; i++) c[i] = (char16)(5); }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device, {properties = cl.QUEUE_PROFILING_ENABLE})

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "init")

-- Create a buffer to hold the data 
local size = NUM_BYTES*cl.sizeof('char')
local buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

-- Set kernel arguments and execute the kernel
cl.set_kernel_arg(kernel, 0, buffer)
cl.set_kernel_arg(kernel, 1, 'int', NUM_BYTES/16)
cl.enqueue_task(queue, kernel)

printf("Bytes: %d, iterations: %d\n", NUM_BYTES, NUM_ITERATIONS)

-- Profile reading the buffer's memory
local total_time = 0
for i=1, NUM_ITERATIONS do
   local mem = cl.malloc(size)
   local prof_event = cl.enqueue_read_buffer(queue, buffer, true, 0, mem:size(), mem:ptr(), nil, true)
   -- Get profiling information 
   local time_start = cl.get_event_profiling_info(prof_event, 'command start')
   local time_end = cl.get_event_profiling_info(prof_event, 'command end')
   total_time = total_time + (time_end - time_start)
   cl.release_event(prof_event)
   mem:free()
end

printf("Average read time: %.0f [ns]\n", total_time/NUM_ITERATIONS)

-- Profile mapping the buffer's memory
total_time = 0
for i=1, NUM_ITERATIONS do
   local ptr, prof_event = cl.enqueue_map_buffer(queue, buffer, true, cl.MAP_READ, 0, size, nil, true)
   local mem = cl.hostmem(size, ptr)
   -- Get profiling information 
   local time_start = cl.get_event_profiling_info(prof_event, 'command start')
   local time_end = cl.get_event_profiling_info(prof_event, 'command end')
   total_time = total_time + (time_end - time_start)
   cl.release_event(prof_event)
   -- Unmap the buffer 
   cl.enqueue_unmap_buffer(queue, buffer, ptr)
   mem:free()
end

printf("Average map time:  %.0f [ns]\n", total_time/NUM_ITERATIONS)

