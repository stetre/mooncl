#!/usr/bin/env lua
-- Division and rounding

local cl = require('mooncl')

--cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

local source = [[
kernel void mod_round(global float *mod_input, 
                      global float *mod_output, 
                      global float4 *round_input,
                      global float4 *round_output)
   {
   /* Use modulus and remainder operators */
   mod_output[0] = fmod(mod_input[0], mod_input[1]);
   mod_output[1] = remainder(mod_input[0], mod_input[1]);
   
   /* Round the input values */
   round_output[0] = rint(*round_input);      
   round_output[1] = round(*round_input);
   round_output[2] = ceil(*round_input);
   round_output[3] = floor(*round_input);
   round_output[4] = trunc(*round_input);   
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
local kernel = cl.create_kernel(program, "mod_round")

local mod_input = {317.0, 23.0}
local round_input = {-6.5, -3.5, 3.5, 6.5}

-- Create buffers to hold input/output data 
local mod_input_mem = cl.malloc('float', mod_input)
local mod_input_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
                     mod_input_mem:size(), mod_input_mem:ptr())
local round_input_mem = cl.malloc('float', round_input)
local round_input_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
         round_input_mem:size(), round_input_mem:ptr())         

local mod_output_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 2*cl.sizeof('float')) 
local round_output_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 5*#round_input*cl.sizeof('float'))

-- Set arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, mod_input_buf)
cl.set_kernel_arg(kernel, 1, mod_output_buf)
cl.set_kernel_arg(kernel, 2, round_input_buf)
cl.set_kernel_arg(kernel, 3, round_output_buf)
cl.enqueue_task(queue, kernel)

-- Read back and print the results 
local mod_output_mem = cl.malloc(2*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, mod_output_buf, true, 0, mod_output_mem:size(), mod_output_mem:ptr()) 
local mod_output = mod_output_mem:read(0, nil, 'float')

local round_output_mem = cl.malloc(20*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, round_output_buf, true, 0, round_output_mem:size(), round_output_mem:ptr()) 
local round_output = round_output_mem:read(0, nil, 'float')
 
printf("fmod(%.1f, %.1f)      = %.1f\n", mod_input[1], mod_input[2], mod_output[1])
printf("remainder(%.1f, %.1f) = %.1f\n\n", mod_input[1], mod_input[2], mod_output[2])
   
printf("Rounding input: %.1f %.1f %.1f %.1f\n", 
         round_input[1], round_input[2], round_input[3], round_input[4])
printf("rint:  %.1f, %.1f, %.1f, %.1f\n", 
         round_output[1], round_output[2], round_output[3], round_output[4])
printf("round: %.1f, %.1f, %.1f, %.1f\n", 
         round_output[5], round_output[6], round_output[7], round_output[8])
printf("ceil:  %.1f, %.1f, %.1f, %.1f\n", 
         round_output[9], round_output[10], round_output[11], round_output[12])
printf("floor: %.1f, %.1f, %.1f, %.1f\n", 
         round_output[13], round_output[14], round_output[15], round_output[16])
printf("trunc: %.1f, %.1f, %.1f, %.1f\n", 
         round_output[17], round_output[18], round_output[19], round_output[20])         

