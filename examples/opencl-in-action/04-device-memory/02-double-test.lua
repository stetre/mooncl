#!/usr/bin/env lua
-- Using double precision floating points

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
/* Enable the double type if available */
#ifdef FP_64
#pragma OPENCL EXTENSION cl_khr_fp64: enable
#endif

kernel void double_test(global float* a, global float* b, global float* out)
   {
#ifdef FP_64
   double c = (double)((*a)/(*b));
   *out = (float)c;
#else /* Use floats if doubles are unavailable */
   *out = (*a)*(*b);
#endif
   }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

print("Address width: " .. cl.get_device_info(device, 'address bits'))

-- Define "FP_64" option if doubles are supported 
local options
if cl.is_extension_supported(device, "cl_khr_fp64") then
   print("The cl_khr_fp64 extension is supported")
   options = "-DFP_64"
else
   print("The cl_khr_fp64 extension is not supported")
end

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device}, options)
local kernel = cl.create_kernel(program, "double_test")

local floatsz = cl.sizeof('float')

-- Create CL buffers to hold input and output data 
local a, b = 6.0, 2.0
local mem = cl.malloc(floatsz)
mem:write(0, 'float', a)
local a_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, floatsz, mem:ptr())
mem:write(0, 'float', b)
local b_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, floatsz, mem:ptr())
local output_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, floatsz)

-- Set arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, a_buf)
cl.set_kernel_arg(kernel, 1, b_buf)
cl.set_kernel_arg(kernel, 2, output_buffer)
cl.enqueue_task(queue, kernel)

-- Read back and print the result 
cl.enqueue_read_buffer(queue, output_buffer, true, 0, floatsz, mem:ptr())
local result = mem:read(0, nil, 'float')
print("The kernel result is " .. result[1])

