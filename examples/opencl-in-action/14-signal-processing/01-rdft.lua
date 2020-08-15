#!/usr/bin/env lua
-- The Discrete Fourier Transform

local cl = require('mooncl')
local fft = require('fft')
-- cl.trace_objects(true)
local function printf(...) io.write(string.format(...)) end
local abs = math.abs

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)
-- Build program and create kernel
local program = cl.create_program_with_sourcefile(context, "rdft.cl")
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, "rdft")

local N = 256 -- no. of signal samples

-- Initialize data with a rectangle function
local input = fft.zero(N)
for i=1, N/4 do input[i] = 1.0 end
local mem = cl.malloc('float', input)
local buf = cl.create_buffer(context, cl.MEM_READ_WRITE|cl.MEM_COPY_HOST_PTR, mem:size(), mem:ptr())

-- Set arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, buf)
local global_size, local_size = (N/2)+1, (N/2)+1
cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {global_size}, {local_size})

-- Read back the result
cl.enqueue_read_buffer(queue, buf, true, 0, mem:size(), mem:ptr())
local output = mem:read(0, nil, 'float')

-- Compute the result with Lua and check it against the kernel's result
local re_output, im_output = fft.fft(input)
local ok = true
if abs(output[1] - re_output[1]) > 0.001 or
   abs(output[2] - re_output[N/2+1]) > 0.001 then
   ok = false
else
   for i=2, N/2-1, 2 do
      if abs(output[i+1] - re_output[i/2+1]) > 0.001 or
         abs(output[i+2] - im_output[i/2+1]) > 0.001 then
         ok = false
         break
      end
   end
end

printf("\nReal-valued DFT check %s.\n\n", ok and "succeeded" or "failed")

