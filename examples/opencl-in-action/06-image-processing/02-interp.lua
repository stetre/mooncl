#!/usr/bin/env lua
-- Image interpolation

local cl = require("mooncl")
local mi = require("moonimage")

local SCALE_FACTOR = 3

-- cl.trace_objects(true)

local source = [[
constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_CLAMP|CLK_FILTER_NEAREST;

kernel void interp(read_only image2d_t src_image, write_only image2d_t dst_image)
   {
   float4 pixel;

   /* Determine input coordinate */
   float2 input_coord = (float2)(get_global_id(0) + (1.0f/(SCALE*2)),
                                 get_global_id(1) + (1.0f/(SCALE*2)));

   /* Determine output coordinate */
   int2 output_coord = (int2)(SCALE*get_global_id(0), SCALE*get_global_id(1));

   /* Compute interpolation */
   for(int i=0; i<SCALE; i++)
      {
      for(int j=0; j<SCALE; j++)
         {
         pixel = read_imagef(src_image, sampler,
                    (float2)(input_coord + (float2)(1.0f*i/SCALE, 1.0f*j/SCALE)));
         write_imagef(dst_image, output_coord + (int2)(i, j), pixel);
         } 
      }
   }
]]

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_source(context, source)
cl.build_program(program, {device}, "-DSCALE="..SCALE_FACTOR)
local kernel = cl.create_kernel(program, "interp")

-- Open input file and read image data
local data, width, height = mi.load("input.png", 'y')
--print(#data, width, height, width*height)

-- Create an input image object and initialize it with pixel data
local imageformat = { channel_order='luminance', channel_type ='unorm int8' }
local imagedesc = { type='image2d', width=width, height=height }
local input_image = cl.create_image(context, cl.MEM_READ_ONLY|cl.MEM_COPY_HOST_PTR, 
                  imageformat, imagedesc, data)
-- Create an image object for output:
local output_width = width*SCALE_FACTOR
local output_height = height*SCALE_FACTOR
local imagedesc = { type='image2d', width=output_width*SCALE_FACTOR, height=output_height }
local output_image = cl.create_image(context, cl.MEM_WRITE_ONLY, imageformat, imagedesc)

-- Set kernel arguments and enqueue kernel for execution
cl.set_kernel_arg(kernel, 0, input_image)
cl.set_kernel_arg(kernel, 1, output_image)
cl.enqueue_ndrange_kernel(queue, kernel, 2, nil, {width, height})

-- Read the output image object 
local mem = cl.malloc(output_width*output_height)
local origin = {0, 0, 0}
local region = {output_width, output_height, 1}
cl.enqueue_read_image(queue, output_image, true, origin, region, 0, 0, mem:ptr())

-- Write the output image to a png file and visualize it 
mi.write_png("output.png", output_width, output_height, 'y', mem:read())
os.execute("eog output.png")

