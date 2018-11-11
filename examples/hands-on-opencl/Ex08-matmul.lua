#!/usr/bin/env lua
--------------------------------------------------------------------------------
-- Program: Matrix Multiplication driver
-- Purpose: This is a driver program to test various ways of computing
--          the product:
--                C  = A * B
--          A and B are set to constant matrices so we
--          can make a quick test of the multiplication.
--
-- Usage:   The matrices are constant matrices, square and the order is
--          set as a constant, ORDER (see mult.h).
--
-- History: Written by Tim Mattson, August 2010
--          Modified by Simon McIntosh-Smith, September 2011
--          Modified by Tom Deakin and Simon McIntosh-Smith, October 2012
--          Ported to C by Tom Deakin, July 2013
--          Modified to assume square matrices by Simon McIntosh-Smith, Sep 2014
--          Updated to use device picker by Tom Deakin, November 2014
--          Ported to Lua by Stefano Trettel, November 2018
--------------------------------------------------------------------------------

local cl = require("mooncl")
local common = require("common")
local mat = require("matrix_lib")
local function printf(...) io.write(string.format(...)) end

-- cl.trace_objects(true) -- uncomment to trace creation/deletion of objects

--------------------------------------------------------------------------------
--  Constants
local ORDER =    1024    -- Order of the square matrices A, B, and C
local N = ORDER
local AVAL =     3.0     -- A elements are constant and equal to AVAL
local BVAL =     5.0     -- B elements are constant and equal to BVAL
local TOL =      0.001   -- tolerance used in floating point comparisons
local DIM =      2       -- Max dim for NDRange
local COUNT =    1       -- number of times to do each multiplication

----------------------------------------------------------------------------------
-- Create device, context, and queue.
----------------------------------------------------------------------------------

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

----------------------------------------------------------------------------------
-- Setup the buffers, initialize matrices, and write them into global memory
----------------------------------------------------------------------------------

local A = mat(N, AVAL)
local B = mat(N, BVAL)

-- Allocate host memory to hold input and output data
local size = N*N*cl.sizeof('float') -- size in bytes of input and output vectors
local h_a = cl.malloc('float', A)   -- input
local h_b = cl.malloc('float', B)   -- input
local h_c = cl.malloc(size)         -- output

-- Allocate device memory to hold input and output data
local d_a = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_a:ptr())
local d_b = cl.create_buffer(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, size, h_b:ptr())
local d_c = cl.create_buffer(context, cl.MEM_WRITE_ONLY, size)

----------------------------------------------------------------------------------
-- OpenCL matrix multiplication ... Naive
----------------------------------------------------------------------------------

do 
   -- Create and build the compute program from the source file
   local program = cl.make_program_with_sourcefile(context, "kernels/C_elem.cl", {device})
   -- Create the compute kernel from the program
   local kernel = cl.create_kernel(program, "mmul")

   printf("\n===== OpenCL, matrix mult, C(i,j) per work item, order %d ======\n",N)

   -- Do the multiplication COUNT times
   for i = 1, COUNT do
      cl.set_kernel_arg(kernel, 0, 'int', N)
      cl.set_kernel_arg(kernel, 1, d_a)
      cl.set_kernel_arg(kernel, 2, d_b)
      cl.set_kernel_arg(kernel, 3, d_c)

      local start_time = cl.now()

      -- Execute the kernel over the entire range of C matrix elements ... computing
      -- a dot product for each element of the product matrix.  The local work
      -- group size is set to nil ... so I'm telling the OpenCL runtime to
      -- figure out a local work group size for me.
      cl.enqueue_ndrange_kernel(queue, kernel, 2, nil, {N, N}, nil)
      cl.finish(queue)
      local run_time = cl.since(start_time)

      cl.enqueue_read_buffer(queue, d_c, true, 0, size, h_c:ptr())
      -- Unpack h_c and use it to initialize our Lua matrix C:
      local C = mat(N)
      C:init(h_c:read(0, nil, 'float'))
      C:results(AVAL, BVAL, TOL, run_time)
   end
end

----------------------------------------------------------------------------------
-- OpenCL matrix multiplication ... C row per work item
----------------------------------------------------------------------------------

do
   -- Create and build the compute program from the source file
   local program = cl.make_program_with_sourcefile(context, "kernels/C_row.cl", {device})

   -- Create the compute kernel from the program
   local kernel = cl.create_kernel(program, "mmul")

   printf("\n===== OpenCL, matrix mult, C row per work item, order %d ======\n",N)

   -- Do the multiplication COUNT times
   for i = 1, COUNT do
      cl.set_kernel_arg(kernel, 0, 'int', N)
      cl.set_kernel_arg(kernel, 1, d_a)
      cl.set_kernel_arg(kernel, 2, d_b)
      cl.set_kernel_arg(kernel, 3, d_c)

      local start_time = cl.now()
   
      -- Execute the kernel over the rows of the C matrix ... computing
      -- a dot product for each element of the product matrix.
      cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {N}, nil)
      cl.finish(queue)
      local run_time = cl.since(start_time)

      cl.enqueue_read_buffer(queue, d_c, true, 0, size, h_c:ptr())
      -- Unpack h_c and use it to initialize our Lua matrix C:
      local C = mat(N)
      C:init(h_c:read(0, nil, 'float'))
      C:results(AVAL, BVAL, TOL, run_time)
   end
end

----------------------------------------------------------------------------------
-- OpenCL matrix multiplication ... C row per work item, A row in private memory
----------------------------------------------------------------------------------
do
   -- Create and build the compute program from the source file
   local program = cl.make_program_with_sourcefile(context, "kernels/C_row_priv.cl", {device})

   -- Create the compute kernel from the program
   local kernel = cl.create_kernel(program, "mmul")

   printf("\n===== OpenCL, matrix mult, C row, A row in priv mem, order %d ======\n",N)

   -- Do the multiplication COUNT times
   for i = 1, COUNT do
      cl.set_kernel_arg(kernel, 0, 'int', N)
      cl.set_kernel_arg(kernel, 1, d_a)
      cl.set_kernel_arg(kernel, 2, d_b)
      cl.set_kernel_arg(kernel, 3, d_c)

      local start_time = cl.now()
   
      -- Execute the kernel over the rows of the C matrix ... computing
      -- a dot product for each element of the product matrix.
      cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {N}, {N/16})
      cl.finish(queue)
      local run_time = cl.since(start_time)

      cl.enqueue_read_buffer(queue, d_c, true, 0, size, h_c:ptr())
      -- Unpack h_c and use it to initialize our Lua matrix C:
      local C = mat(N)
      C:init(h_c:read(0, nil, 'float'))
      C:results(AVAL, BVAL, TOL, run_time)
   end
end

----------------------------------------------------------------------------------
-- OpenCL matrix multiplication ... C row per work item, A row pivate, B col local
----------------------------------------------------------------------------------
do
   -- Create and build the compute program from the source file
   local program = cl.make_program_with_sourcefile(context, "kernels/C_row_priv_bloc.cl", {device})

   -- Create the compute kernel from the program
   local kernel = cl.create_kernel(program, "mmul")

   printf("\n===== OpenCL, mat mult, C row, priv A, B cols loc, order %d ======\n",N)

   -- Do the multiplication COUNT times
   for i = 1, COUNT do
      cl.set_kernel_arg(kernel, 0, 'int', N)
      cl.set_kernel_arg(kernel, 1, d_a)
      cl.set_kernel_arg(kernel, 2, d_b)
      cl.set_kernel_arg(kernel, 3, d_c)
      cl.set_kernel_arg(kernel, 4, cl.sizeof('float')*N)

      local start_time = cl.now()
   
      -- Execute the kernel over the rows of the C matrix ... computing
      -- a dot product for each element of the product matrix.
      cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {N}, {N/16})
      cl.finish(queue)
      local run_time = cl.since(start_time)

      cl.enqueue_read_buffer(queue, d_c, true, 0, size, h_c:ptr())
      -- Unpack h_c and use it to initialize our Lua matrix C:
      local C = mat(N)
      C:init(h_c:read(0, nil, 'float'))
      C:results(AVAL, BVAL, TOL, run_time)
   end
end

----------------------------------------------------------------------------------
-- OpenCL matrix multiplication ... blocked
----------------------------------------------------------------------------------
do
   -- Create and build the compute program from the source file
   local program = cl.make_program_with_sourcefile(context, "kernels/C_block_form.cl", {device})

   -- Create the compute kernel from the program
   local kernel = cl.create_kernel(program, "mmul")

   printf("\n===== Parallel matrix mult (blocked), order %d on device ======\n",N)

   -- Do the multiplication COUNT times
   for i = 1, COUNT do
      -- Work-group computes a block of C.  This size is also set
      -- in a #define inside the kernel function.  Note this blocksize
      -- must evenly divide the matrix order */
      local blocksize = 16
      cl.set_kernel_arg(kernel, 0, 'int', N)
      cl.set_kernel_arg(kernel, 1, d_a)
      cl.set_kernel_arg(kernel, 2, d_b)
      cl.set_kernel_arg(kernel, 3, d_c)
      cl.set_kernel_arg(kernel, 4, cl.sizeof('float')*blocksize*blocksize)
      cl.set_kernel_arg(kernel, 5, cl.sizeof('float')*blocksize*blocksize)

      local start_time = cl.now()
   
      -- Execute the kernel over the rows of the C matrix ... computing
      -- a dot product for each element of the product matrix.
      cl.enqueue_ndrange_kernel(queue, kernel, 2, nil, {N,N}, {blocksize, blocksize})
      cl.finish(queue)
      local run_time = cl.since(start_time)

      cl.enqueue_read_buffer(queue, d_c, true, 0, size, h_c:ptr())
      -- Unpack h_c and use it to initialize our Lua matrix C:
      local C = mat(N)
      C:init(h_c:read(0, nil, 'float'))
      C:results(AVAL, BVAL, TOL, run_time)
   end
end

