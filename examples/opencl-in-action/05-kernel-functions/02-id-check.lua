#!/usr/bin/env lua
-- Testing work-item/work-group ids.

local cl = require('mooncl')

-- cl.trace_objects(true)

local source = [[
kernel void id_check(global float *output)
   {
   /* Get work-item/work-group information */
   size_t global_id_0 = get_global_id(0);
   size_t global_id_1 = get_global_id(1);
   size_t global_size_0 = get_global_size(0);
   size_t offset_0 = get_global_offset(0);
   size_t offset_1 = get_global_offset(1);
   size_t local_id_0 = get_local_id(0);
   size_t local_id_1 = get_local_id(1);

   /* Determine the index of this work-item in the output array */
   int index_0 = global_id_0 - offset_0;
   int index_1 = global_id_1 - offset_1;
   int index = index_1 * global_size_0 + index_0;
   
   /* Compose a float woth the ids of this work-item and write it to the output */
   float f = global_id_0 * 10.0f + global_id_1 * 1.0f;
   f += local_id_0 * 0.1f + local_id_1 * 0.01f;

   output[index] = f;
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
local kernel = cl.create_kernel(program, "id_check")

-- Work-space parameters
local work_dim = 2            -- dimensions of the work-space
local Nx, Ny = 6, 4
local global_size = {Nx, Ny}  -- size of the work-space
local global_offset = {3, 5}  -- where the global ids start from
local local_size = {3, 2}     -- size of the work-groups

-- Create a write-only buffer to hold the output data 
local bufsize = Nx*Ny*cl.sizeof('float')
local buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, bufsize)

-- Set argument and enqueue kernel
cl.set_kernel_arg(kernel, 0, buffer)
cl.enqueue_ndrange_kernel(queue, kernel, work_dim, global_offset, global_size, local_size)

-- Read back and print the result 
local mem = cl.malloc(bufsize)
cl.enqueue_read_buffer(queue, buffer, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'float')

for i=1, #result, 6 do
   io.write(string.format("%.2f   %.2f   %.2f   %.2f   %.2f   %.2f\n", 
            table.unpack(result, i, i+5)))
end

print("Legend: XY.xy -> X=global_id(0), Y=global_id(1), x=local_id(0), y=local_id(1)")
