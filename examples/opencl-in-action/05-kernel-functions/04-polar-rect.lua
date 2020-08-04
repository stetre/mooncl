#!/usr/bin/env lua
-- Rectilinear and polar coordinates

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void polar_rect(global float4 *r_coords, global float4 *angles,
                       global float4 *x_coords, global float4 *y_coords)
   {
   /* Compute coordinates on unit circle */
   *y_coords = sincos(*angles, x_coords);
   /* Compute coordinates for different radii */
   *x_coords *= *r_coords;
   *y_coords *= *r_coords;
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
local kernel = cl.create_kernel(program, "polar_rect")

-- Input data 
local pi = math.pi
local r_coords = {2, 1, 3, 4}
local angles = {3*pi/8, 3*pi/4, 4*pi/3, 11*pi/6}

-- Create input and output buffers 
local r_coords_mem = cl.malloc('float', r_coords)
local r_coords_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR,
         r_coords_mem:size(), r_coords_mem:ptr())
local angles_mem = cl.malloc('float', angles)
local angles_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR,
         angles_mem:size(), angles_mem:ptr())   

local x_coords_buf = cl.create_buffer(context, cl.MEM_READ_WRITE, 4*cl.sizeof('float'))
local y_coords_buf = cl.create_buffer(context, cl.MEM_READ_WRITE, 4*cl.sizeof('float'))
 
-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, r_coords_buf)
cl.set_kernel_arg(kernel, 1, angles_buf)
cl.set_kernel_arg(kernel, 2, x_coords_buf)
cl.set_kernel_arg(kernel, 3, y_coords_buf)
cl.enqueue_task(queue, kernel)

-- Read back and print the results 
local mem = cl.malloc(4*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, x_coords_buf, true, 0, mem:size(), mem:ptr())
local x_coords = mem:read(0, nil, 'float')
cl.enqueue_read_buffer(queue, y_coords_buf, true, 0, mem:size(), mem:ptr())
local y_coords = mem:read(0, nil, 'float')
for i=1, 4 do
   io.write(string.format("(%6.3f, %6.3f)\n", x_coords[i], y_coords[i]))
end

