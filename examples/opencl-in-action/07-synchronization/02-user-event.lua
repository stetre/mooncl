#!/usr/bin/env lua
-- Stalling commands with user events

local cl = require('mooncl')
local glfw = require('moonglfw') -- just for the sleep() function

-- cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

local source = [[
kernel void change_sign(global float4 *v)
   { *v *= -1.0f; }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device, 
   { properties = cl.QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE })
   
-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "change_sign")
   
-- Input data
local mem = cl.malloc('float', { 0, 1, 2, 3 })
-- Create a buffer to hold input and output data
local buffer = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Create a user event to be used as wait event
local user_event = cl.create_user_event(context)

-- Set argument and enqueue kernel for execution, forcing it to wait on user_event to
-- be 'complete' before executing. Also, generate an event associated with this command.
cl.set_kernel_arg(kernel, 0, buffer)
local kernel_event = cl.enqueue_task(queue, kernel, {user_event}, true)
-- Enqueue a command to read the buffer, forcing it to wait on the event associated with
-- the kernel execution command. Again, generate an event associated with the command.
-- Note also that this call is made non-blocking, otherwise the application would hang
-- (the read command would wait for the kernel to complete, the kernel would wait for the
-- application to set the user event status, and the application would wait for the read
-- command to complete, resulting in a deadlock).
local read_event = cl.enqueue_read_buffer(queue, buffer, false, 0, mem:size(), mem:ptr(), 
         {kernel_event}, true)

-- Set callbacks for the events associated with the commands.
cl.set_event_callback(kernel_event, 'complete')
cl.set_event_callback(read_event, 'complete')

-- Sleep for 1 second to show that the commands have not executed, then prompt the user.
print("sleeping...")
glfw.sleep(1)
print('kernel complete?', cl.check_event_callback(kernel_event, 'complete'))
print('read complete?  ', cl.check_event_callback(read_event, 'complete'))
printf("Press ENTER to continue.\n")
io.read()

-- Set the status of user_event to 'complete' so to trigger the execution of the kernel
-- command (that is waiting on it). At its completion, the read command will execute too.
cl.set_user_event_status(user_event, 'complete')

local kernel_complete, read_complete = false, false
while not read_complete do
   kernel_complete = cl.check_event_callback(kernel_event, 'complete')
   read_complete = cl.check_event_callback(read_event, 'complete')
   print('kernel complete?', kernel_complete)
   print('  read complete?', read_complete)
end
   
if type(read_complete) == 'number' then error('event error '..(read_complete)) end

printf("data: %4.2f, %4.2f, %4.2f, %4.2f\n", table.unpack(mem:read(0, nil, 'float')))

