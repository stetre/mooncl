#!/usr/bin/env lua
-- Selecting component content

local cl = require('mooncl')

local function printf(...) io.write(string.format(...)) end

-- cl.trace_objects(true)

local source = [[
kernel void select_test(global float4 *s1, global uchar2 *s2)
   {
   /* select usage */
   int4 mask1 = (int4)(-1, 0, -1, 0);
   float4 input1 = (float4)(0.25f, 0.5f, 0.75f, 1.0f);
   float4 input2 = (float4)(1.25f, 1.5f, 1.75f, 2.0f);
   *s1 = select(input1, input2, mask1); 
   
   /* bitselect usage */
   uchar2 mask2 = (uchar2)(0xAA, 0x55);
   uchar2 input3 = (uchar2)(0x0F, 0x0F);
   uchar2 input4 = (uchar2)(0x33, 0x33);
   *s2 = bitselect(input3, input4, mask2); 
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
local kernel = cl.create_kernel(program, "select_test")

-- Create a write-only buffer to hold the output data 
local select1_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 4*cl.sizeof('float'))
local select2_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 2*cl.sizeof('uchar'))
         
-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, select1_buf)
cl.set_kernel_arg(kernel, 1, select2_buf)
cl.enqueue_task(queue, kernel)

-- Read and print the result 
local mem1 = cl.malloc(4*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, select1_buf, true, 0, mem1:size(), mem1:ptr())
local mem2 = cl.malloc(2*cl.sizeof('uchar'))
cl.enqueue_read_buffer(queue, select2_buf, true, 0, mem2:size(), mem2:ptr()) 

local select1 = mem1:read(0, nil, 'float')
local select2 = mem2:read(0, nil, 'uchar')

printf("select: ")
for i=1,3 do printf("%.2f, ", select1[i]) end
printf("%.2f\n", select1[4])
printf("bitselect: %X, %X\n", select2[1], select2[2])

