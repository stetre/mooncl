#!/usr/bin/env lua
-- Numerical reduction with multiple kernels and completion on the device

local cl = require('mooncl')

-- cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device, {properties=cl.QUEUE_PROFILING_ENABLE})

-- Build program
local program = cl.create_program_with_sourcefile(context, "reduction_complete.cl")
cl.build_program(program, {device})

-- Initialize data 
local N=1048576
local data = {}
for i=1, N do data[i] = i-1 end
local actual_sum = N*(N-1)/2 -- = 0 + 1 + 2 ... + N 

-- Determine local size 
local max_wg_size = cl.get_device_info(device, 'max work group size')
local global_size = N/4
local local_size  = max_wg_size

-- Create data buffer 
local data_mem = cl.malloc('float', data)
local data_buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_USE_HOST_PTR, data_mem:size(), data_mem:ptr())
local sum_buf = cl.create_buffer(context, cl.MEM_WRITE_ONLY, cl.sizeof('float'))

-- Create kernels 
local vector_kernel = cl.create_kernel(program, "reduction_vector")
local complete_kernel = cl.create_kernel(program, "reduction_complete")

-- Set arguments for vector kernel 
cl.set_kernel_arg(vector_kernel, 0, data_buf)
cl.set_kernel_arg(vector_kernel, 1, local_size * 4 * cl.sizeof('float'))

-- Perform the first stage of the reduction
printf("Global size = %d\t(first stage)\n", global_size)
local start_event = cl.enqueue_ndrange_kernel(queue, vector_kernel, 1, nil, {global_size}, {local_size}, nil, true)
global_size = global_size/local_size

-- Perform any successive stage of the reduction
while global_size > local_size do
   printf("Global size = %d\n", global_size)
   cl.enqueue_ndrange_kernel(queue, vector_kernel, 1, nil, {global_size}, {local_size})
   global_size = global_size/local_size
end

-- Set arguments and execute the complete_kernel 
cl.set_kernel_arg(complete_kernel, 0, data_buf)
cl.set_kernel_arg(complete_kernel, 1, local_size * 4 * cl.sizeof('float'))
cl.set_kernel_arg(complete_kernel, 2, sum_buf)
printf("Global size = %d\t(completion)\n", global_size)
local end_event = cl.enqueue_ndrange_kernel(queue, complete_kernel, 1, nil, {global_size}, nil, nil, true)

-- Finish processing the queue and get profiling information 
cl.finish(queue)
local total_time = cl.get_event_profiling_info(end_event, 'command end')
                 - cl.get_event_profiling_info(start_event, 'command start')

-- Read the result and check it
local sum_mem = cl.malloc(cl.sizeof('float'))
cl.enqueue_read_buffer(queue, sum_buf, true, 0, sum_mem:size(), sum_mem:ptr())
local sum = sum_mem:read(0, nil, 'float')[1]
printf("Check %s\n", (sum == actual_sum and "passed" or "failed"))
printf("Total time = %.0f [us]\n", total_time/1000)

