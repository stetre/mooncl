#!/usr/bin/env lua
-- A general bitonic sort

local cl = require('mooncl')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Input data 
local ascending, descending = 0, -1
local dir = ascending
if arg[1] == 'descending' then dir=descending end
local N = 1048576 -- no. of elements in the array to be sorted
math.randomseed(os.time())
local data = {}
for i=1, N do data[i] = math.random(2^31-1) end
local mem = cl.malloc('float', data)
local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Build program and create kernels 
local program = cl.create_program_with_sourcefile(context, "bsort.cl")
cl.build_program(program, {device})
local kernel_init = cl.create_kernel(program, "bsort_init")
local kernel_stage_0 = cl.create_kernel(program, "bsort_stage_0")
local kernel_stage_n = cl.create_kernel(program, "bsort_stage_n")
local kernel_merge = cl.create_kernel(program, "bsort_merge")
local kernel_merge_last = cl.create_kernel(program, "bsort_merge_last")

-- Determine the maximum work-group size 
local wg_size = cl.get_kernel_work_group_info(kernel_init, device, 'work group size')
local local_size = math.tointeger(2^math.floor(math.log(wg_size, 2)))

-- Set arguments for kernels
local locsz = 8*local_size*cl.sizeof('float')
cl.set_kernel_arg(kernel_init, 0, buf)
cl.set_kernel_arg(kernel_init, 1, locsz)
cl.set_kernel_arg(kernel_stage_0, 0, buf)
cl.set_kernel_arg(kernel_stage_0, 1, locsz)
cl.set_kernel_arg(kernel_stage_n, 0, buf)
cl.set_kernel_arg(kernel_stage_n, 1, locsz)
cl.set_kernel_arg(kernel_merge, 0, buf)
cl.set_kernel_arg(kernel_merge, 1, locsz)
cl.set_kernel_arg(kernel_merge, 3, 'int', dir)
cl.set_kernel_arg(kernel_merge_last, 0, buf)
cl.set_kernel_arg(kernel_merge_last, 1, locsz)
cl.set_kernel_arg(kernel_merge_last, 2, 'int',  dir)

-- Enqueue initial sorting kernel 
local global_size = N/8
if global_size < local_size then local_size = global_size end
cl.enqueue_ndrange_kernel(queue, kernel_init, 1, nil, {global_size}, {local_size}) 
-- Execute further stages 
local num_stages = math.tointeger(global_size/local_size)
local high_stage = 2
while high_stage < num_stages do
   cl.set_kernel_arg(kernel_stage_0, 2, 'int', high_stage)
   cl.set_kernel_arg(kernel_stage_n, 3, 'int', high_stage)
   local stage = high_stage 
   while stage > 1 do
      cl.set_kernel_arg(kernel_stage_n, 2, 'int', stage)
      cl.enqueue_ndrange_kernel(queue, kernel_stage_n, 1, nil, {global_size}, {local_size}) 
      stage = stage >> 1
   end
   cl.enqueue_ndrange_kernel(queue, kernel_stage_0, 1, nil, {global_size}, {local_size}) 
   high_stage = high_stage << 1
end

-- Perform the bitonic merge 
local stage = num_stages 
while stage > 1 do
   cl.set_kernel_arg(kernel_merge, 2, 'int', stage)
   cl.enqueue_ndrange_kernel(queue, kernel_merge, 1, nil, {global_size}, {local_size}) 
   stage = stage >> 1
end
cl.enqueue_ndrange_kernel(queue, kernel_merge_last, 1, nil, {global_size}, {local_size}) 

-- Read back the result
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
data = mem:read(0, nil, 'float')

-- Check and display result
local ok = true
if dir == ascending then
   for i=2, N do if data[i] < data[i-1] then ok = false break end end
elseif dir == descending then
   for i=2, N do if data[i] > data[i-1] then ok = false break end end
end

printf("Local size: %u\n", local_size)
printf("Global size: %u\n", global_size)
print("Bitonic sort ".. (ok and "succeeded." or "failed."))

