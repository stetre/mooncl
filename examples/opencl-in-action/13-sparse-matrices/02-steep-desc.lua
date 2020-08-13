#!/usr/bin/env lua
-- Solving a sparse matrix system Ax=b with the Steepest Descent method

-- @@ NOTE: This example needs some debugging (the original too: for example,
-- the r_length variable in the shader is not initialized so no iterations are
-- performed if its initial value is less than 0.01)

local cl = require('mooncl')
local mmio = require('mmio')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

local mmfile = arg[1] or "bcsstk05.mtx"

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)
-- Build program and create kernel
local program = cl.create_program_with_sourcefile(context, "steep_desc.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "steep_desc")

-- Read matrix from file:
local matrix = assert(mmio.read(mmfile))
local matrix_data = mmio.fulldata(matrix, true, 'row')
local num_rows = matrix.M
local num_values = #matrix_data
-- Now matrix_data contains all the nonzero elements, in row-major order, with
-- each element represented as { i, j, value } where i and j are 0-based.
-- Extract indices and values in separate tables
local rows, cols, values = {}, {}, {}
for _, v in ipairs(matrix_data) do
   local i, j, value = table.unpack(v)
   rows[#rows+1] = i
   cols[#cols+1] = j
   values[#values+1] = value
end

-- Initialize the b vector
local b_vec = {}
math.randomseed(os.time())
for i = 1, num_rows do b_vec[i] = math.random(2^31-1) end

-- Pack data and encapsulate into hostmem objects
local rows_mem = cl.malloc('int', rows)
local cols_mem = cl.malloc('int', cols)
local values_mem = cl.malloc('float', values)
local b_vec_mem = cl.malloc('float', b_vec)

-- Create buffers and initialize them
local rows_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR,
   rows_mem:size(), rows_mem:ptr())
local cols_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
   cols_mem:size(), cols_mem:ptr())
local values_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR,
   values_mem:size(), values_mem:ptr())
local b_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR,
   b_vec_mem:size(), b_vec_mem:ptr())
local result_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, 2*cl.sizeof('float'))

-- Set arguments and enqueue kernel for execution
local global_size, local_size = num_rows, num_rows
cl.set_kernel_arg(kernel, 0, 'int', num_rows)
cl.set_kernel_arg(kernel, 1, 'int', num_values)
cl.set_kernel_arg(kernel, 2, num_rows*cl.sizeof('float'))
cl.set_kernel_arg(kernel, 3, num_rows*cl.sizeof('float'))
cl.set_kernel_arg(kernel, 4, num_rows*cl.sizeof('float'))
cl.set_kernel_arg(kernel, 5, rows_buf)
cl.set_kernel_arg(kernel, 6, cols_buf)
cl.set_kernel_arg(kernel, 7, values_buf)
cl.set_kernel_arg(kernel, 8, b_buf)
cl.set_kernel_arg(kernel, 9, result_buf)
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {global_size}, {local_size})

-- Read and print the results
local mem = cl.malloc(2*cl.sizeof('float'))
cl.enqueue_read_buffer(queue, result_buf, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'float')
printf("After %d iterations, the residual length is %f.\n", result[1], result[2]);

