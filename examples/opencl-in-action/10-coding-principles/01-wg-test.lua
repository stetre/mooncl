#!/usr/bin/env lua
-- Obtaining kernel and device information

local cl = require('mooncl')

local function printf(...) io.write(string.format(...)) end

-- Read program name and kernel name from input arguments:
if not arg[1] or not arg[2] then
   printf("Usage:   %s <program_file> <kernel_name>\n", arg[0])
   printf("Example: %s blank.cl blank\n\n", arg[0])
   os.exit(1)
end
local kernelfile = arg[1]
local kernelname = arg[2]

-- cl.trace_objects(true)

-- Create device, context, and command queue
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]
local context = cl.create_context(platform, {device})
local queue = cl.create_command_queue(context, device)

-- Build program and create kernel 
local program = cl.create_program_with_sourcefile(context, kernelfile)
cl.build_program(program, {device})
local kernel = cl.create_kernel(program, kernelname)

-- Access device properties 
local device_name = cl.get_device_info(device, 'name')
local local_mem = cl.get_device_info(device, 'local mem size')
   
-- Access kernel/work-group properties
local wg_size =  cl.get_kernel_work_group_info(kernel, device, 'work group size')
local wg_multiple = cl.get_kernel_work_group_info(kernel, device, 'preferred work group size multiple')
local local_usage = cl.get_kernel_work_group_info(kernel, device, 'local mem size')
local private_usage = cl.get_kernel_work_group_info(kernel, device, 'private mem size')

-- Print information 
printf("\nFor the '%s' kernel running on the '%s' device,\n"..
      "the maximum work-group size is %u and the work-group multiple is %u.\n"..
      "The kernel uses %u bytes of local memory out of a maximum of %u bytes.\n"..
      "It uses %u bytes of private memory.\n\n",
      kernelname, device_name, wg_size, wg_multiple, local_usage, local_mem, private_usage)

