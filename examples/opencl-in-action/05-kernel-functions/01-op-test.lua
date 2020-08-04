#!/usr/bin/env lua
-- Operator usage

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void op_test(global int4 *output)
   {
   int4 vec = (int4)(1, 2, 3, 4);
   
   /* Adds 4 to every element of vec */
   vec += 4;
   
   /* Sets the third element to 0. Doesn't change the other elements (-1 in hexadecimal = 0xffffffff) */
   if(vec.s2 == 7)
      vec &= (int4)(-1, -1, 0, -1);
   
   /* Sets the first element to -1, the second to 0 */
   vec.s01 = vec.s23 < 7; 
   
   /* Divides the last element by 2 until it is less than or equal to 7 */
   while(vec.s3 > 7 && (vec.s0 < 16 || vec.s1 < 16))
      vec.s3 >>= 1; 

   *output = vec;
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
local kernel = cl.create_kernel(program, "op_test")

-- Create a write-only buffer to hold the output data 
local test_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 4*cl.sizeof('int'))

-- Set argument and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, test_buffer)
cl.enqueue_task(queue, kernel)

-- Read back and print the result 
local mem = cl.malloc(4*cl.sizeof('int'))
cl.enqueue_read_buffer(queue, test_buffer, true, 0, mem:size(), mem:ptr())
local test = mem:read(0, nil, 'int')
print(table.concat(test, ', '))

