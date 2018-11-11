#!/usr/bin/env lua
--------------------------------------------------------------------------------
-- Numeric integration to estimate pi
-- 
-- History: Written by Tim Mattson, May 2010 
--          Ported to the C++ Wrapper API by Benedict R. Gaster, September 2011
--          Updated by Tom Deakin and Simon McIntosh-Smith, October 2012
--          Ported back to C by Tom Deakin, July 2013
--          Updated by Tom Deakin, October 2014
--          Ported to Lua by Stefano Trettel, November 2018
--------------------------------------------------------------------------------

local cl = require("mooncl")
local common = require("common")
local function printf(...) io.write(string.format(...)) end

-- cl.trace_objects(true) -- uncomment to trace creation/deletion of objects

--------------------------------------------------------------------------------
--  Constants
local INSTEPS = 512*512*512
local ITERS = 262144

-- Create device, context, and queue.
local device_index = common.parse_arguments()

-- Get list of devices
local devices = common.get_device_list()
-- Check device index in range
if device_index < 0 or device_index > #devices then
   printf("Invalid device index (try '--list')\n")
   os.exit(true)
end

local device = devices[device_index]
local platform = device:platform()
printf("\nUsing OpenCL device: %s\n", cl.get_device_info(device, 'name'))

-- Create a compute context
local context = cl.create_context(platform, {device})

-- Create a command queue
local queue = cl.create_command_queue(context, device)

-- Create and build the compute program from the source file
local program = cl.make_program_with_sourcefile(context, "kernels/pi_ocl.cl", {device})
-- Create the compute kernel from the program
local kernel = cl.create_kernel(program, "pi")

-- Find kernel work-group size
local work_group_size = cl.get_kernel_work_group_info(kernel, device, 'work group size')
-- Now that we know the size of the work-groups, we can set the number of
-- work-groups, the actual number of steps, and the step size
local nwork_groups = INSTEPS/(work_group_size*ITERS)
if nwork_groups < 1 then
   nwork_groups = cl.get_device_info(device, 'max compute units')
   work_group_size = INSTEPS / (nwork_groups * ITERS)
end
local nsteps = work_group_size * ITERS * nwork_groups
local step_size = 1.0/nsteps
printf(" %d work-groups of size %d. %d Integration steps\n", nwork_groups, work_group_size, nsteps)

-- Allocate host memory to hold partial sum
local h_psum = cl.malloc(cl.sizeof('float')*nwork_groups)
-- Allocate device memory to hold partial sum
local d_partial_sums = cl.create_buffer(context, cl.MEM_WRITE_ONLY, h_psum:size())

-- Set kernel arguments
cl.set_kernel_arg(kernel, 0, 'int', ITERS)
cl.set_kernel_arg(kernel, 1, 'float', step_size)
cl.set_kernel_arg(kernel, 2, cl.sizeof('float')*work_group_size)
cl.set_kernel_arg(kernel, 3, d_partial_sums)

-- Execute the kernel over the entire range of our 1D input data set
-- using the maximum number of work items for this device
local rtime = cl.now()
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {nsteps/ITERS}, {work_group_size})

-- Read back the partial sums and unpack them:
cl.enqueue_read_buffer(queue, d_partial_sums, true, 0, h_psum:size(), h_psum:ptr())
local psum = h_psum:read(0, nil, 'float')
-- complete the sum and compute the final integral value on the host
local pi_res = 0.0
for i = 1, nwork_groups do
   pi_res = pi_res + psum[i]
end
pi_res = pi_res * step_size

rtime = cl.since(rtime)

printf("\nThe calculation ran in %f seconds\n", rtime)
printf(" pi = %f for %d steps\n", pi_res, nsteps)
