#!/usr/bin/env lua
-- Sphere animation

local cl = require("mooncl")
local gl = require("moongl")
local glfw = require("moonglfw")

local kernelsource = [[
kernel void sphere(global float4* vertices, float tick)
   {
   int longitude = get_global_id(0)/16;
   int latitude = get_global_id(0) % 16;

   float sign = -2.0f * (longitude % 2) + 1.0f;
   float phi = 2.0f * M_PI_F * longitude/16 + tick;
   float theta = M_PI_F * latitude/16;

   vertices[get_global_id(0)].x = RADIUS * sin(theta) * cos(phi);
   vertices[get_global_id(0)].y = RADIUS * sign * cos(theta);
   vertices[get_global_id(0)].z = RADIUS * sin(theta) * sin(phi);
   vertices[get_global_id(0)].w = 1.0f;
   }
]]

local vertshader = [[
#version 330 core
in  vec4 in_coords;

void main(void)
   { gl_Position = in_coords; }
]]

local fragshader = [[
#version 330 core
out vec4 out_color;

void main(void)
   { out_color = vec4(0.0, 0.0, 1.0, 1.0); }
]]

local NUM_VERTICES = 256

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
local window = glfw.create_window(300, 300, "Sphere")
glfw.make_context_current(window)
gl.init()
gl.enable('depth test')
gl.line_width(3)
gl.clear_color(1.0, 1.0, 1.0, 1.0)

-- Create and compile shaders 
local prog, vsh, fsh = gl.make_program_s('vertex', vertshader, 'fragment', fragshader)
gl.bind_attrib_location(prog, 0, "in_coords")
gl.bind_attrib_location(prog, 1, "in_color")
gl.delete_shaders(vsh, fsh)
gl.use_program(prog)

-- Initialize OpenCL ----------------------------------------------------------
local device, platform = select_device()
local context = create_context(platform, {device}, window)
local program = cl.create_program_with_source(context, kernelsource)
cl.build_program(program, {device}, "-DRADIUS=0.75")
local queue = cl.create_command_queue(context, device)
local kernel = cl.create_kernel(program, "sphere")

-- Create GL buffer ------------------------------------------------------------
local vao = gl.new_vertex_array()
local vbo = gl.new_buffer('array')
gl.buffer_data('array', 4*NUM_VERTICES*gl.sizeof('float'), 'dynamic draw')
gl.vertex_attrib_pointer(0, 4, 'float', false, 0, 0) 
gl.enable_vertex_attrib_array(0)

-- Create CL buffer from GL buffer ---------------------------------------------
local vertex_buffer = cl.create_from_gl_buffer(context, cl.MEM_WRITE_ONLY, vbo)

-- Execute the kernel ----------------------------------------------------------
local function execute_kernel(tick)
   gl.finish() -- complete OpenGL processing 
   cl.enqueue_acquire_gl_objects(queue, {vertex_buffer})
   cl.set_kernel_arg(kernel, 0, vertex_buffer)
   cl.set_kernel_arg(kernel, 1, 'float', tick)
   local kernel_event = cl.enqueue_ndrange_kernel(queue, kernel, 1, nil, {NUM_VERTICES}, nil, nil, true) 
   cl.wait_for_events({kernel_event})
   cl.enqueue_release_gl_objects(queue, {vertex_buffer})
   cl.finish(queue)
   cl.release_event(kernel_event)
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

-- Endless loop ----------------------------------------------------------------
local tick = 0
collectgarbage()
collectgarbage('stop')
while not glfw.window_should_close(window) do
   glfw.wait_events_timeout(1/60)
   gl.clear('color', 'depth')
   gl.use_program(prog)
   execute_kernel(tick)
   gl.bind_vertex_array(vao)
   gl.draw_arrays('line loop', 0, NUM_VERTICES)
   tick = tick + 0.0001
   gl.unbind_vertex_array()
   gl.use_program(0)
   glfw.swap_buffers(window)
   collectgarbage()
end

