#!/usr/bin/env lua
-- Simple image processing

local cl = require('mooncl')
local mi = require('moonimage')

--cl.trace_objects(true)

local source = [[
constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST; 

kernel void simple_image(read_only image2d_t src_image, write_only image2d_t dst_image)
   {
   /* Compute value to be subtracted from each pixel */
   uint offset = get_global_id(1)*0x40 + get_global_id(0)*0x10;

   /* Read pixel value */
   int2 coord = (int2)(get_global_id(0), get_global_id(1));
   uint4 pixel = read_imageui(src_image, sampler, coord);

   /* Subtract offset from pixel */
   pixel.x -= offset;

   /* Write new pixel value to output */
   write_imageui(dst_image, coord, pixel);
   }
]]

-- Create device, context, and command_queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device}, options)
local kernel = cl.create_kernel(program, "simple_image")

-- Open input file and read image data
local data, width, height= mi.load("blank.png", 'y')

-- Create an input image object and initialize it with pixels data:
local imageformat = { channel_order='luminance', channel_type ='unsigned int8' }
local imagedesc = { type='image2d', width=width, height=height }
local input_image = cl.create_image(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
                  imageformat, imagedesc, data)
-- Create an image object for output, with the same properties as the input:
local output_image = cl.create_image(context, cl.MEM_WRITE_ONLY, imageformat, imagedesc)

-- Set arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, input_image)
cl.set_kernel_arg(kernel, 1, output_image)
cl.enqueue_ndrange_kernel(queue, kernel, 2, nil, {height, width})

-- Read the image object 
local origin = {0, 0, 0}
local region = {width, height, 1}
local mem = cl.malloc(#data)
cl.enqueue_read_image(queue, output_image, true, origin, region, 0, 0, mem:ptr())

-- Write the output image to a png file and view image
local outdata = mem:read(0, nil, 'uchar')
mi.write_png("output.png", width, height, 'y', mem:read())
os.execute("eog output.png")

