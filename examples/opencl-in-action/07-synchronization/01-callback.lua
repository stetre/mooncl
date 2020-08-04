#!/usr/bin/env lua
-- Host notification

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void initialize(global float *buffer)
/* Initialize all the buffer elements to a known value, so that we
   can later check if the initialization was completed correctly */
   {
   float4 five_vector = (float4)(5.0);
   for(int i=0; i<1024; i++) 
      vstore4(five_vector, i, buffer);
   }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "initialize")

local data_size = 4096*cl.sizeof('float')
local mem = cl.malloc(data_size)

-- Create a write-only buffer to hold the output data 
local data_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, data_size)

-- Set argument and enqueue the kernel for execution, then enqueue a command
-- to read the buffer. In both cases, set the ge argument ('generate event')
-- to true, so that an event associated with the command is generated and
-- returned. We use this event below to set a few callbacks that the OpenCL
-- driver is expected to call when the command execution status changes.
cl.set_kernel_arg(kernel, 0, data_buffer)
local kernel_event = cl.enqueue_task(queue, kernel, nil, true)
local read_event = cl.enqueue_read_buffer(queue, data_buffer, false, 0, mem:size(), mem:ptr(), nil, true)

-- Set event callbacks. In MoonCL, the callbacks are executed only at the C level,
-- to avoid slowing down the OpenCL implementation. The Lua application can use the
-- cl.check_event_callback() function to poll the status of a registered callback.
cl.set_event_callback(kernel_event, 'submitted')
cl.set_event_callback(kernel_event, 'running') 
cl.set_event_callback(kernel_event, 'complete') 
cl.set_event_callback(read_event, 'submitted') 
cl.set_event_callback(read_event, 'running') 
cl.set_event_callback(read_event, 'complete') 

-- Keep polling all the callbacks until both commands have reached the 'complete' status
local kernel_complete, read_complete = false, false
while not (kernel_complete and read_complete) do
   kernel_submitted = cl.check_event_callback(kernel_event, 'submitted')
   kernel_running = cl.check_event_callback(kernel_event, 'running')
   kernel_complete = cl.check_event_callback(kernel_event, 'complete') 
   read_submitted = cl.check_event_callback(read_event, 'submitted')
   read_running = cl.check_event_callback(read_event, 'running')
   read_complete = cl.check_event_callback(read_event, 'complete') 
   print('kernel submitted', kernel_submitted)
   print('kernel running  ', kernel_running)
   print('kernel complete ', kernel_complete)
   print('  read submitted', read_submitted)
   print('  read running  ', read_running)
   print('  read complete ', read_complete)
end

-- Check if the data was initialized as expected.
local data = mem:read(0, nil, 'float')
local ok = true
for i = 1, 4096 do
   if data[i] ~= 5.0 then
      ok = false
      break
   end
end
if ok then
   print("The data has been initialized successfully.")
 else
   print("The data has not been initialized successfully.")
end

-- Note that the application is in charge of releasing the generated events
-- (here we do it explicitly even if in this case it is not necessary, since we're exiting)
cl.release_event(kernel_event)
cl.release_event(read_event)

