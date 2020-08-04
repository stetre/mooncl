#!/usr/bin/env lua
-- Shuffling vector components

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void shuffle_test(global float8 *s1, global char16 *s2)
   {
   /* Execute the first example */
   uint8 mask1 = (uint8)(1, 2, 0, 1, 3, 1, 2, 3);
   float4 input = (float4)(0.25f, 0.5f, 0.75f, 1.0f);
   *s1 = shuffle(input, mask1); 
   
   /* Execute the second example */
   uchar16 mask2 = (uchar16)(6, 10, 5, 2, 8, 0, 9, 14, 7, 5, 12, 3, 11, 15, 1, 13);
   char8 input1 = (char8)('l', 'o', 'f', 'c', 'a', 'u', 's', 'f');
   char8 input2 = (char8)('f', 'e', 'h', 't', 'n', 'n', '2', 'i');
   *s2 = shuffle2(input1, input2, mask2); 
   }
]]

local function printf(...) io.write(string.format(...)) end
 
-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "shuffle_test")

-- Create a write-only buffer to hold the output data 
local shuffle1_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 8*cl.sizeof('float'))
local shuffle2_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 16*cl.sizeof('char'))
         
-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, shuffle1_buf)
cl.set_kernel_arg(kernel, 1, shuffle2_buf)
cl.enqueue_task(queue, kernel)

-- Read back and print the results
local mem1 = cl.malloc(8*cl.sizeof('float'))
local mem2 = cl.malloc(16*cl.sizeof('char'))
cl.enqueue_read_buffer(queue, shuffle1_buf, true, 0, mem1:size(), mem1:ptr())
cl.enqueue_read_buffer(queue, shuffle2_buf, true, 0, mem2:size(), mem2:ptr())
local shuffle1 = mem1:read(0, nil, 'float')
local shuffle2 = mem2:read(0, nil, 'char')
   
printf("Shuffle1: ")
for i=1,7 do printf("%.2f, ", shuffle1[i]) end
printf("%.2f\n", shuffle1[8])
   
printf("Shuffle2: ")
for i=1,16 do printf("%c", shuffle2[i]) end
-- Alternatively:
printf("\nShuffle2: %s\n", mem2:read())

