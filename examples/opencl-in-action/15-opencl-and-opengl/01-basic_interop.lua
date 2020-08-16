#!/usr/bin/env lua
-- Basic OpenGL/OpenCL interoperatibility example

local cl = require("mooncl")
local gl = require("moongl")
local glfw = require("moonglfw")

-- Kernel (OpenCL C)
local kernelsource = [[
kernel void basic_interop(global float4* first_coords, 
      global float4* first_colors, global float4* second_coords, 
      global float4* second_colors, global float4* third_coords, 
      global float4* third_colors)
   {
   first_coords[0] = (float4)(-0.15f, -0.15f,  1.00f, -0.15f);
   first_coords[1] = (float4)( 0.15f,  1.00f,  0.15f,  0.15f);
   first_coords[2] = (float4)( 1.00f,  0.15f, -0.15f,  1.00f);

   first_colors[0] = (float4)(0.00f, 0.00f, 0.00f, 0.25f);
   first_colors[1] = (float4)(0.00f, 0.00f, 0.50f, 0.00f);
   first_colors[2] = (float4)(0.00f, 0.75f, 0.00f, 0.00f);

   second_coords[0] = (float4)(-0.30f, -0.30f,  0.00f, -0.30f);
   second_coords[1] = (float4)( 0.30f,  0.00f,  0.30f,  0.30f);
   second_coords[2] = (float4)( 0.00f,  0.30f, -0.30f,  0.00f);

   second_colors[0] = (float4)(0.00f, 0.00f, 0.00f, 0.00f);
   second_colors[1] = (float4)(0.25f, 0.00f, 0.00f, 0.50f);
   second_colors[2] = (float4)(0.00f, 0.00f, 0.75f, 0.00f);

   third_coords[0] = (float4)(-0.45f, -0.45f, -1.00f, -0.45f);
   third_coords[1] = (float4)( 0.45f, -1.00f,  0.45f,  0.45f);
   third_coords[2] = (float4)(-1.00f,  0.45f, -0.45f, -1.00f);

   third_colors[0] = (float4)(0.00f, 0.00f, 0.00f, 0.00f);
   third_colors[1] = (float4)(0.00f, 0.25f, 0.00f, 0.00f);
   third_colors[2] = (float4)(0.50f, 0.00f, 0.00f, 0.75f);
   }
]]

-- Vertex shader (GLSL)
local vertshader = [[
#version 330 core
in  vec3 in_coords;
in  vec3 in_color;
out vec3 new_color;

void main(void)
   {
   new_color = in_color;
   mat3x3 rot_matrix = mat3x3(0.707, 0.641, -0.299,
                             -0.707, 0.641, -0.299,
                             -0.000, 0.423,  0.906);
   vec3 coords = rot_matrix * in_coords;
   gl_Position = vec4(coords, 1.0);
   }
]]

-- Fragment shader (GLSL)
local fragshader = [[
#version 330 core
in  vec3 new_color;
out vec4 out_color;

void main(void)
   {
   vec3 tmp_color = new_color + vec3(0.25f, 0.25f, 0.25f);
   out_color = vec4(tmp_color, 1.0);
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
-- Create window and GL context with GLFW:
glfw.version_hint(3, 3, 'core')
local window = glfw.create_window(300, 300, "Basic CL/GL Interoperability")
glfw.make_context_current(window)

gl.init()
gl.enable('depth test')
gl.clear_color(1.0, 1.0, 1.0, 1.0)

-- Create and compile shaders 
local prog, vs, fs = gl.make_program_s('vertex', vertshader, 'fragment', fragshader)
gl.bind_attrib_location(prog, 0, "in_coords")
gl.bind_attrib_location(prog, 1, "in_color")
gl.delete_shaders(vs, fs)
gl.use_program(prog)

-- Initialize OpenCL ----------------------------------------------------------
local device, platform = select_device()
local context = create_context(platform, {device}, window)
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program, {device})
local queue = cl.create_command_queue(context, device)
local kernel = cl.create_kernel(program, "basic_interop")

-- Create GL buffers -----------------------------------------------------------

local vao = {} -- 3 vertex array objects - one for each square 
local vbo = {} -- 6 vertex buffer objects - one for each set of coordinates and colors 

-- Size for VBOs (each will contain 3 vec4):
local vbsize = 12*gl.sizeof('float') 

-- VAO for first square:
vao[1] = gl.new_vertex_array()
-- VBO for coordinates
vbo[1] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(0, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(0)
-- VBO for colors
vbo[2] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(1, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(1)

-- VAO for second square:
vao[2] = gl.new_vertex_array()
-- VBO for coordinates
vbo[3] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(0, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(0)
-- VBO for colors
vbo[4] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(1, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(1)

-- VAO for third square:
vao[3] = gl.new_vertex_array()
-- VBO for coordinates
vbo[5] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(0, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(0)
-- VBO for colors
vbo[6] = gl.new_buffer('array')
gl.buffer_data('array', vbsize, 'dynamic draw')
gl.vertex_attrib_pointer(1, 3, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(1)

-- Create CL buffers from GL buffers -------------------------------------------
local buffers = {}
for i=1,#vbo do
   buffers[i] = cl.create_from_gl_buffer(context, cl.MEM_WRITE_ONLY, vbo[i])
end

-- Execute the kernel ----------------------------------------------------------
gl.finish() -- Complete OpenGL processing (is this really needed here?)
-- Pass the buffers as argument to the kernel and execute the kernel
for i=1,#buffers do
   cl.set_kernel_arg(kernel, i-1, buffers[i])
end
cl.enqueue_acquire_gl_objects(queue, buffers)
local kernel_event = cl.enqueue_task(queue, kernel, nil, true)
cl.wait_for_events({kernel_event})
cl.enqueue_release_gl_objects(queue, buffers)
cl.finish(queue)
cl.release_event(kernel_event)
  
-- Set GLFW callback functions -------------------------------------------------
glfw.set_window_size_callback(window, function(window, w, h)
   gl.viewport(0, 0, w, h)
end)

glfw.set_key_callback(window, function(window, key, scancode, action)
   if (key == 'escape' or key == 'q') and action == 'press' then
      glfw.set_window_should_close(window, true) -- will cause exiting the loop
   end
end)

-- Event loop ------------------------------------------------------------------
while not glfw.window_should_close(window) do
   glfw.wait_events_timeout(1/60)

   gl.clear('color', 'depth')
   gl.use_program(prog)

   gl.bind_vertex_array(vao[3])
   gl.draw_arrays('triangle fan', 0, 4)
   gl.bind_vertex_array(vao[2])
   gl.draw_arrays('triangle fan', 0, 4)
   gl.bind_vertex_array(vao[1])
   gl.draw_arrays('triangle fan', 0, 4)
   gl.unbind_vertex_array()

   gl.use_program(0)
   glfw.swap_buffers(window)
end

