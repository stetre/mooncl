#!/usr/bin/env lua
-- Testing atomic operations

local cl = require('mooncl')

-- cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

local source = [[
kernel void increment(global int* x)
   {
   local int a, b;
   a = 0; 
   b = 0;
   a++;            /* Increment with the ++ operator (which is not atomic) */
   atomic_inc(&b); /* Increment with atomic add */
   x[0] = a;
   x[1] = b;
   }
]]
-- Note that a and b are shared by the work-items belonging to the same work group,
-- i.e. by 4 work-items. So they will be incremented 4 times, even if the total number
-- of work-items is 8. Put it differently, we have 2 work-groups of 4 work-items each,
-- and each work-group acts on a different instance of a and of b. When writing to the
-- output buffer x, though, all the 8 work-items write to the same locations, without
-- synchronization. So the values that we will read back will be the last written ones
-- (which may come from any of the 8 work-items).
-- The final value of a will be 1 because the 4 work items increment it in parallel (i.e.,
-- each work-item increments a=0). The final value of b, on the other side, will be 4
-- because the atomic_inc() causes the 4 work-items to increment it sequentially, one
-- after the other (note that this ensures correctness but also slows down the execution).

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "increment")

-- Create a buffer to hold output data
local size = 2*cl.sizeof('int')
local buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

-- Set argument and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, buffer)
cl.enqueue_ndrange_kernel(queue, kernel, 1, {0}, {8}, {4})

-- Read back the buffer 
local mem = cl.malloc(size)
cl.enqueue_read_buffer(queue, buffer, true, 0, mem:size(), mem:ptr())
local data = mem:read(0, nil, 'int')

printf("a = %d (incremented with the ++ operator)\n", data[1])
printf("b = %d (incremented with atomic add)\n", data[2])

