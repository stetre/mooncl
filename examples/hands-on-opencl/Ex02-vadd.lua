#!/usr/bin/env lua
-- Elementwise addition of two vectors (c = a + b)
--
-- History: Written by Tim Mattson, December 2009
--          Updated by Tom Deakin and Simon McIntosh-Smith, October 2012
--          Updated by Tom Deakin, July 2013
--          Updated by Tom Deakin, October 2014
--          Ported to Lua by Stefano Trettel, November 2018

local cl = require("mooncl")
local common = require("common")
local function printf(...) io.write(string.format(...)) end

-- cl.trace_objects(true) -- uncomment to trace creation/deletion of objects

-- pick up device type from compiler command line or from the default type
local DEVICE = arg[1] and cl.devicetypeflags(arg[1]) or cl.devicetypeflags('default')

local TOL = 0.001   -- tolerance used in floating point comparisons
local LENGTH = 1024 -- length of vectors a, b, and c

--------------------------------------------------------------------------------
-- Kernel:  vadd
-- Purpose: Compute the elementwise sum c = a+b
-- Input:   a and b float vectors of length count
-- Output:  c float vector of length count holding the sum a + b
local KernelSource = [[
kernel void vadd(global float* a, global float* b, global float* c, const unsigned int count)
   {
   int i = get_global_id(0);
   if(i < count)
      c[i] = a[i] + b[i];
   }
]]
--------------------------------------------------------------------------------

-- Fill vectors a and b with random float values
local a, b = {}, {}
math.randomseed(os.time())
for i = 1, LENGTH do
   a[i], b[i] = math.random(), math.random()
end

-- Allocate host memory to hold input and output data
local size = LENGTH*cl.sizeof('float') -- size in bytes of input and output vectors
local h_a = cl.malloc('float', a)  -- initialized with the contents of vector a
local h_b = cl.malloc('float', b)  -- initialized with the contents of vector b
local h_c = cl.malloc(size) -- c vector (a+b) returned from the compute device

-- Set up platform and GPU device

-- Get all platforms
local platforms = cl.get_platform_ids()
assert(#platforms>0, "Found 0 platforms!\n")

-- Secure a GPU
local device, platform
for i=1, #platforms do
   device = cl.get_device_ids(platforms[i], DEVICE)[1]
   if device then
      platform = platforms[i]
      break
   end
end

assert(device, "Cannot find a suitable device")
common.output_device_info(device, true)

-- Create a compute context
local context = cl.create_context(platform, {device})

-- Create a command queue
local queue = cl.create_command_queue(context, device)

-- Create the compute program from the source buffer
local program = cl.create_program_with_source(context, KernelSource)

-- Build the program
cl.build_program(program, {device})

-- Create the compute kernel from the program
local ko_vadd = cl.create_kernel(program, "vadd")

-- Create the input (a, b) and output (c) arrays in device memory
local d_a = cl.create_buffer(context, cl.MEM_READ_ONLY,  size)
local d_b = cl.create_buffer(context, cl.MEM_READ_ONLY,  size)
local d_c = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

-- Write a and b vectors into compute device memory
cl.enqueue_write_buffer(queue, d_a, true, 0, h_a:size(), h_a:ptr())
cl.enqueue_write_buffer(queue, d_b, true, 0, h_b:size(), h_b:ptr())

-- Set the arguments to our compute kernel
cl.set_kernel_arg(ko_vadd, 0, d_a)
cl.set_kernel_arg(ko_vadd, 1, d_b)
cl.set_kernel_arg(ko_vadd, 2, d_c)
cl.set_kernel_arg(ko_vadd, 3, 'uint', LENGTH)

local rtime = cl.now()

-- Execute the kernel over the entire range of our 1d input data set
-- letting the OpenCL runtime choose the work-group size
cl.enqueue_ndrange_kernel(queue, ko_vadd, 1, nil, {LENGTH}, nil)

-- Wait for the commands to complete before stopping the timer
cl.finish(queue)

rtime = cl.since(rtime)
printf("\nThe kernel ran in %.6f seconds\n",rtime)

-- Read back the results from the compute device
cl.enqueue_read_buffer(queue, d_c, true, 0, h_c:size(), h_c:ptr())
local c = h_c:read(0, nil, 'float') -- unpack the result vector

-- Test the results
local correct = 0 -- number of correct results
local tol2 = TOL^2
for i = 1, LENGTH do
   local tmp = a[i] + b[i] - c[i] -- deviation of expected and output result
   if tmp*tmp < tol2 then -- correct if square deviation is less than tolerance squared
      correct = correct + 1
   else
      printf(" tmp %f a %f b %f c %f \n",tmp, a[i], b[i], c[i])
   end
end

-- Summarize results
printf("C = A+B:  %d out of %d results were correct.\n", correct, LENGTH)

-- Cleanup at exit is done automatically by MoonCL.

