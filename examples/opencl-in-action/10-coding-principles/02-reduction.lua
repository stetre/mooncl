#!/usr/bin/env lua
-- Numerical reduction using scalars and using vectors

local cl = require('mooncl')

local function printf(...) io.write(string.format(...)) end

-- cl.trace_objects(true)

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device, {properties = cl.QUEUE_PROFILING_ENABLE})

-- Build program
local program = cl.create_program_with_sourcefile(context, "reduction.cl")
cl.build_program(program, {device})

-- Input data 
local N = 1048576 -- no of elements in data
local data = {}
for i = 1, N do data[i] = i-1 end
local actual_sum = N*(N-1)/2 -- = 0 + 1 + 2 ... + N 
local data_mem = cl.malloc('float', data)
local data_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
                  data_mem:size(), data_mem:ptr())

-- Determine max work group size 
local max_wg_size = cl.get_device_info(device, cl.DEVICE_MAX_WORK_GROUP_SIZE)

local function execute(kernelname, vec_size)
   local global_size = N/vec_size
   local local_size = max_wg_size
   local num_groups = global_size/local_size
   -- Create output buffer
   local mem = cl.malloc(num_groups*cl.sizeof('float'))
   local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())
   -- Create kernel 
   local kernel = cl.create_kernel(program, kernelname)
   -- Set arguments and enqueue kernel for execution
   cl.set_kernel_arg(kernel, 0, data_buf)
   cl.set_kernel_arg(kernel, 1, local_size*vec_size*cl.sizeof('float'))
   cl.set_kernel_arg(kernel, 2, buf)
   local ev = cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {global_size}, {local_size}, nil, true)
   cl.finish(queue)
   -- Get profiling information 
   local time_start = cl.get_event_profiling_info(ev, 'command start')
   local time_end = cl.get_event_profiling_info(ev, 'command end')
   local total_time = time_end - time_start
   -- Read back the partial sums and add them up
   cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
   local partial_sums = mem:read(0, nil, 'float')
   local sum = 0
   for j = 1, #partial_sums do sum = sum + partial_sums[j] end
   -- Check result 
   printf("\n%s: check %s.\n", kernelname, sum==actual_sum and "passed" or "failed")
   printf("total time = %.0f [us]\n", total_time/1000.0)
   cl.release_buffer(buf)
   cl.free(mem)
   cl.release_event(ev)
   cl.release_kernel(kernel)
end

execute("reduction_scalar", 1)
execute("reduction_vector", 4)

-- For comparison, let's do the sum sequentially in Lua
local t = cl.now()
local sum = 0
for _, x in ipairs(data) do sum = sum + x end
t = cl.since(t) -- seconds
printf("\nLua: check %s.\n", sum==actual_sum and "passed" or "failed")
printf("total time = %.0f [us]\n\n", t*1e6)

