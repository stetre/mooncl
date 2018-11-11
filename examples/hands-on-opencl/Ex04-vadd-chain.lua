#!/usr/bin/env lua
-- Elementwise addition of two vectors at a time in a chain (C=A+B; D=C+E; F=D+G)
--
-- History: Initial version based on vadd.c, written by Tim Mattson, June 2011
--          Ported to C++ Wrapper API by Benedict Gaster, September 2011
--          Updated to C++ Wrapper API v1.2 by Tom Deakin and Simon McIntosh-Smith, October 2012
--          Ported back to C by Tom Deakin, July 2013
--          Updated by Tom Deakin, October 2014
--          Ported to Lua by Stefano Trettel, November 2018
--
--------------------------------------------------------------------------------

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


-- Fill input vectors with random float values
local a, b, e, g = {}, {}, {}, {}
math.randomseed(os.time())
for i = 1, LENGTH do
   a[i], b[i] = math.random(), math.random()
   e[i], g[i] = math.random(), math.random()
end

-- Allocate host memory to hold input and output data
local size = LENGTH*cl.sizeof('float') -- size in bytes of input and output vectors
local h_a = cl.malloc('float', a)   -- input
local h_b = cl.malloc('float', b)   -- input
local h_e = cl.malloc('float', e)   -- input
local h_f = cl.malloc(size)         -- output
local h_g = cl.malloc('float', g)   -- input

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

-- Create the input arrays in device memory
-- NB: we copy the host pointers here too
local d_a = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_a:ptr())
local d_b = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_b:ptr())
local d_e = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_e:ptr())
local d_g = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_g:ptr())
    
-- Create the output arrays in device memory
local d_c = cl.create_buffer(context, cl.MEM_READ_WRITE, size)
local d_d = cl.create_buffer(context, cl.MEM_READ_WRITE, size)
local d_f = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

-- Enqueue kernel - first time
-- Set the arguments to our compute kernel
cl.set_kernel_arg(ko_vadd, 0, d_a)
cl.set_kernel_arg(ko_vadd, 1, d_b)
cl.set_kernel_arg(ko_vadd, 2, d_c)
cl.set_kernel_arg(ko_vadd, 3, 'uint', LENGTH)
   
-- Execute the kernel over the entire range of our 1d input data set
-- letting the OpenCL runtime choose the work-group size
cl.enqueue_ndrange_kernel(queue, ko_vadd, 1, nil, {LENGTH}, nil)

-- Enqueue kernel - second time
-- Set different arguments to our compute kernel
cl.set_kernel_arg(ko_vadd, 0, d_e)
cl.set_kernel_arg(ko_vadd, 1, d_c)
cl.set_kernel_arg(ko_vadd, 2, d_d)
    
-- Enqueue the kernel again
cl.enqueue_ndrange_kernel(queue, ko_vadd, 1, nil, {LENGTH}, nil)

-- Enqueue kernel - third time
-- Set different (again) arguments to our compute kernel
cl.set_kernel_arg(ko_vadd, 0, d_g)
cl.set_kernel_arg(ko_vadd, 1, d_d)
cl.set_kernel_arg(ko_vadd, 2, d_f)

-- Enqueue the kernel again
cl.enqueue_ndrange_kernel(queue, ko_vadd, 1, nil, {LENGTH}, nil)

-- Read back the results from the compute device
cl.enqueue_read_buffer(queue, d_f, true, 0, h_f:size(), h_f:ptr())
local f = h_f:read(0, nil, 'float') -- unpack the result vector

-- Test the results
local correct = 0 -- number of correct results
local tol2 = TOL^2
for i = 1, LENGTH do
   local tmp = a[i] + b[i] + e[i] + g[i] - f[i] -- deviation of expected and output result
   if tmp*tmp < tol2 then -- correct if square deviation is less than tolerance squared
      correct = correct + 1
   else
      printf(" tmp %f a %f b %f e %f g %f f %f\n",tmp, a[i], b[i], e[i], g[i], f[i])
   end
end

-- Summarize results
printf("C = A+B+E+G:  %d out of %d results were correct.\n", correct, LENGTH)

-- Cleanup at exit is done automatically by MoonCL.

