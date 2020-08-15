#!/usr/bin/env lua
-- The Fast Fourier Transform

local cl = require('mooncl')
local fft = require('fft')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end
local abs, max = math.abs, math.max

-- Returns the maximum power of 2 that is less than n:
local function maxpow2(n) return 2^math.floor(math.log(n, 2)) end

local N = 8192 -- no. of samples (complex numbers
local dir = arg[1] or 'forward' -- 'forward' (FFT) or 'inverse' (IFFT)
assert(dir=='forward' or dir=='inverse', "invalid direction")

--- Initialize data
local data = {}  -- N samples, with real and imaginary parts interleaved 
math.randomseed(os.time())
for i=1, N do
   data[#data+1] = math.random()
   data[#data+1] = math.random()
end
local mem = cl.malloc('float', data)

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)
-- Build program and create kernels
local program = cl.create_program_with_sourcefile(context, "fft.cl")
cl.build_program(program, {device})
local init_kernel = cl.create_kernel(program, "fft_init")
local stage_kernel = cl.create_kernel(program, "fft_stage")
local scale_kernel = cl.create_kernel(program, "fft_scale")

-- Create buffers
local input_buf = cl.create_buffer(context, cl.MEM_READ_ONLY|cl.MEM_USE_HOST_PTR, mem:size(), mem:ptr())
local output_buf = cl.create_buffer(context, cl.MEM_READ_WRITE, mem:size())

-- Determine maximum work-group size
local local_size =cl.get_kernel_work_group_info(init_kernel, device, 'work group size')
local_size = maxpow2(local_size)

-- Determine local memory size
local local_mem_size = cl.get_device_info(device, 'local mem size')
local_mem_size = maxpow2(local_mem_size)

-- Set kernel arguments
local points_per_group = local_mem_size/(2*cl.sizeof('float'))
if points_per_group > N then points_per_group = N end
cl.set_kernel_arg(init_kernel, 0, input_buf)
cl.set_kernel_arg(init_kernel, 1, output_buf)
cl.set_kernel_arg(init_kernel, 2, local_mem_size)
cl.set_kernel_arg(init_kernel, 3, 'uint', points_per_group)
cl.set_kernel_arg(init_kernel, 4, 'uint', N)
cl.set_kernel_arg(init_kernel, 5, 'int', dir == 'forward' and 1 or -1)

-- Enqueue init kernel
local global_size = (N/points_per_group)*local_size
-- print(global_size, local_size, local_mem_size, points_per_group)
cl.enqueue_ndrange_kernel(queue, init_kernel, 1, nil, {global_size}, {local_size})

-- Enqueue further stages of the FFT
if N > points_per_group then
   cl.set_kernel_arg(stage_kernel, 0, output_buf)
   cl.set_kernel_arg(stage_kernel, 2, 'uint', points_per_group)
   cl.set_kernel_arg(stage_kernel, 3, 'int', dir == 'forward' and 1 or -1)
   local stage = 2
   while stage <= N/points_per_group do
      cl.set_kernel_arg(stage_kernel, 1, 'uint', stage)
      cl.enqueue_ndrange_kernel(queue, stage_kernel, 1, nil, {global_size}, {local_size})
      stage = stage << 1
   end
end

-- Scale values if performing the inverse FFT
if dir == 'inverse' then 
   cl.set_kernel_arg(scale_kernel, 0, output_buf)
   cl.set_kernel_arg(scale_kernel, 1, 'uint', points_per_group)
   cl.set_kernel_arg(scale_kernel, 2, 'uint', N)
   cl.enqueue_ndrange_kernel(queue, scale_kernel, 1, nil, {global_size}, {local_size})
end

-- Read the results
cl.enqueue_read_buffer(queue, output_buf, true, 0, mem:size(), mem:ptr())
local result = mem:read(0, nil, 'float')

-- Compute accurate values to check the result against
local re_data, im_data = {}, {} -- separated real and imaginary parts of input data
for i=1, N do
   re_data[#re_data+1] = data[2*i-1]
   im_data[#im_data+1] = data[2*i]
end
local f = dir == 'forward' and fft.fft or fft.ifft
local re_output, im_output = f(re_data, im_data)
-- Compute and display error
local err = 0.0
for i=1, N do
   err = err + abs(re_output[i] - result[2*i-1]) / max(abs(re_output[i]), 0.0001)
   err = err + abs(im_output[i] - result[2*i])   / max(abs(im_output[i]), 0.0001)
end
err = err/(N*2)
printf("\n%u-point %sFFT completed with %.9f average relative error.\n\n",
   N, dir=='forward' and "" or "I", err);

