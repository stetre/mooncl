#!/usr/bin/env lua
-- Image filtering and texturing

local cl = require("mooncl")
local gl = require("moongl")
local glfw = require("moonglfw")
local mi = require("moonimage")

local sharpen = arg[1] == 'sharpen'
local kernelsourcefile = sharpen and "texture_filter.cl" or "transp_filter.cl"
if not sharpen then
   print("To apply the sharpening filter, run as:\n$ "..arg[0].." sharpen")
end

local vshader = [[
#version 330 core
in  vec3 in_coords;
in  vec2 in_texcoords;
out vec2 texcoords;
void main(void) {
   texcoords = in_texcoords;
   gl_Position = vec4(in_coords, 1.0);
}
]]

local fshader = [[
#version 330 core
uniform sampler2D tex;
in vec2 texcoords;
out vec4 new_color;
void main() {
   vec3 color = vec3(texture(tex, texcoords.st));
   new_color = vec4(color, 1.0);
}
]]

local function select_device()
-- Search for a device in a platform that supports CL/GL interoperation.
   local platforms = cl.get_platform_ids()
   for _, platform in ipairs(platforms) do
      if cl.is_extension_supported(platform, "cl_khr_gl_sharing") then 
         local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
         return device, platform
      end
   end
   error("Cannot find OpenCL device that supports cl_khr_gl_sharing")
end

local function create_context(platform, devices, window)
-- Create a CL context tied to the window's GL context.
   local glcontext, system = glfw.get_context(window)
   local contextprops = { gl_context = glcontext }
   if system == "glx" then
      contextprops.glx_display = glfw.get_x11_display(window)
-- else if system == "wgl" then
--    contextprops.wgl_hdc = glfw.get_ @@?!?!
   else
      error("System '"..system.." is not supported by this example")
   end
   return cl.create_context(platform, devices, contextprops)
end

-- Initialize OpenGL ----------------------------------------------------------
glfw.version_hint(3, 3, 'core')
local window = glfw.create_window(233, 90, "Texture Filter")
glfw.make_context_current(window)
gl.init()
gl.enable('depth test')
gl.clear_color(1.0, 1.0, 1.0, 1.0)

-- Create and compile shaders 
local prog, vs, fs = gl.make_program_s('vertex', vshader, 'fragment', fshader)
gl.bind_attrib_location(prog, 0, "in_coords")
gl.bind_attrib_location(prog, 1, "in_texcoords")
gl.delete_shaders(vs, fs)
gl.use_program(prog)

-- Initialize OpenCL ----------------------------------------------------------
local device, platform = select_device()
local context = create_context(platform, {device}, window)
--program = cl.create_program_with_sourcefile(context, "transp_filter.cl")
local program = cl.create_program_with_sourcefile(context, kernelsourcefile)
cl.build_program(program, {device})
local queue = cl.create_command_queue(context, device)
local kernel = cl.create_kernel(program, "texture_filter")

-- Read pixel data
local data, width, height = mi.load("car.png", 'y')
-- Create the input image object from the PNG data
local in_texture = cl.create_image(context, cl.MEM_READ_ONLY | cl.MEM_COPY_HOST_PTR, 
   { channel_order = 'r', channel_type = 'unsigned int8' }, 
   { type = 'image2d', width = width, height =height },
   data)

-- Create GL texture -----------------------------------------------------------
local texture = gl.new_texture('2d')
gl.pixel_store('unpack alignment', 1)
gl.texture_parameter('2d', 'wrap s', 'repeat')
gl.texture_parameter('2d', 'wrap t', 'repeat')
gl.texture_parameter('2d', 'mag filter', 'linear')
gl.texture_parameter('2d', 'min filter', 'linear')

-- Create GL buffers -----------------------------------------------------------
local vertex_coords = gl.pack('float', {
   -1.0, -1.0, 0.0,
   -1.0,  1.0, 0.0,
    1.0,  1.0, 0.0,
    1.0, -1.0, 0.0
})

local tex_coords = gl.pack('float', {
   0.0, 1.0,
   0.0, 0.0,
   1.0, 0.0,
   1.0, 1.0
})

local vao = gl.new_vertex_array()
-- VBO to hold vertex coordinates
local vbo1 = gl.new_buffer('array')
gl.buffer_data('array', vertex_coords, 'static draw')
gl.vertex_attrib_pointer(0, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(0)
-- VBO to hold texture coordinates
local vbo2 = gl.new_buffer('array')
gl.buffer_data('array', tex_coords, 'static draw')
gl.vertex_attrib_pointer(1, 2, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(1)
gl.unbind_vertex_array()

-- Create and configure pixel buffer
local pbo = gl.new_buffer('array')
gl.buffer_data('array', width*height, 'static draw')
gl.unbind_buffer('array')

-- Create CL buffer from GL buffer ---------------------------------------------
local out_buffer = cl.create_from_gl_buffer(context, cl.MEM_WRITE_ONLY, pbo)

-- Execute the kernel ----------------------------------------------------------
local function execute_kernel()
   -- Complete OpenGL processing 
   gl.finish()
   -- Execute the kernel 
   cl.enqueue_acquire_gl_objects(queue, {out_buffer})
   cl.set_kernel_arg(kernel, 0, in_texture)
   cl.set_kernel_arg(kernel, 1, out_buffer)
   local kernel_event = cl.enqueue_ndrange_kernel(queue, kernel, 2, nil, 
                              {width, height}, nil, nil, true) 
   cl.wait_for_events({kernel_event})
   cl.enqueue_release_gl_objects(queue, {out_buffer})
   cl.finish(queue)
   cl.release_event(kernel_event)
   -- Copy pixels from pbo to texture
   gl.bind_buffer('pixel unpack', pbo)
   gl.texture_image('2d', 0, 'red', 'red', 'ubyte', 0, width, height)
   gl.active_texture(0)
end

-- Set callback functions ------------------------------------------------------
glfw.set_window_size_callback(window, function(window, w, h)
   gl.viewport(0, 0, w, h)
end)

glfw.set_key_callback(window, function(window, key, scancode, action)
   if (key == 'escape' or key == 'q') and action == 'press' then
      glfw.set_window_should_close(window, true) -- will cause exiting the loop
   end
end)

-- Main loop -------------------------------------------------------------------
execute_kernel()

collectgarbage()
collectgarbage('stop')
while not glfw.window_should_close(window) do
   glfw.wait_events_timeout(1/60)
   gl.clear('color', 'depth')
   gl.use_program(prog)
   gl.bind_vertex_array(vao)
   gl.bind_texture('2d', texture)
   gl.draw_arrays('triangle fan', 0, 4)
   gl.unbind_vertex_array()
   gl.use_program(0)
   glfw.swap_buffers(window)
   collectgarbage('step')
end

