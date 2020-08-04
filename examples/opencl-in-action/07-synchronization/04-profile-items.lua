#!/usr/bin/env lua
-- Profiling data partitioning

local cl = require('mooncl')

-- cl.trace_objects(true)

local function printf(...) io.write(string.format(...)) end

local NUM_INTS = 4096       -- size of the data to process (in no. of integers)
local NUM_ITERATIONS = 2000 -- number of iterations

local source = [[
kernel void init(global int4 *x, int num_ints)
   {
   /* Compute the number of vectors to be initialized by this work item. */
   int num_vectors = num_ints/(4*get_global_size(0));
   x += get_global_id(0) * num_vectors;
   for(int i=0; i<num_vectors; i++)
      {
      x[i] += 1;
      x[i] *= 2;
      x[i] /= 3;
      }
   }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device, {properties = cl.QUEUE_PROFILING_ENABLE})

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "init")

-- Input data 
local data = {}
for i= 1, NUM_INTS do data[i] = i-1 end
local mem = cl.malloc('int', data)

-- Create a buffer to hold data 
local buffer = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Set kernel arguments
cl.set_kernel_arg(kernel, 0, buffer)
cl.set_kernel_arg(kernel, 1, 'int', NUM_INTS)


-- Profile the execution time of the kernel using different numbers of work items
-- (i.e. different global work sizes). The data to be processed is always the same size.
-- By increasing the number of work-items each work-item has to process a smaller chunk
-- of data, and as long as the device is able to executed them in parallel, this results
-- in a faster execution of the kernel.
printf("Work-items \tAverage execution time [ns]\n")
local num_items = 1 -- number of items, i.e. global work size
while num_items <= 2048 do
   local total_time = 0
      for i=1, NUM_ITERATIONS do
      -- Enqueue kernel for execution
      local prof_event = cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {num_items}, nil, nil, true)
      cl.finish(queue)
      -- Get profiling information 
      local time_start = cl.get_event_profiling_info(prof_event, 'command start')
      local time_end = cl.get_event_profiling_info(prof_event, 'command end')
      total_time = total_time + time_end - time_start
      cl.release_event(prof_event)
   end
   printf("%3.0d \t\t%.0f\n", num_items, total_time/NUM_ITERATIONS)
   num_items = num_items*2
end
   
