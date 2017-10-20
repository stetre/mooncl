#!/usr/bin/env lua
-- MoonCL example: kernel_work_group_info.lua
--
-- Example:
cl = require("mooncl")

filename = arg[1]
kernel_name = arg[2]

if not filename or not kernel_name then
   print("\nPrint kernel information.")
   print("Usage:   $ "..arg[0] .. " [program_filename] [kernel_name]")
   print("Example: $ "..arg[0] .. " hello.cl myadd\n")
   os.exit()
end

-- Select platform and device, create context and command_queue
platform = cl.get_platform_ids()[1]
device = cl.get_device_ids(platform, cl.DEVICE_TYPE_GPU)[1]
context = cl.create_context(platform, {device})
queue = cl.create_command_queue(context, device)

-- Create program and build it
file, errmsg = io.open(filename, 'r')
assert(file, errmsg)
source = file:read('a')
file:close()
program = cl.create_program_with_source(context, source)

cl.build_program(program, {device})

-- Create kernel 
kernel = cl.create_kernel(program, kernel_name)

-- Get and print info
print()
print("kernel name: " .. kernel_name)
print("file: " .. filename)
print("device: " .. cl.get_device_info(device, "name"))

print("kernel info:")
name = "function name"
print("- ".. name ..": " .. cl.get_kernel_info(kernel, name))

name = "num args"
print("- ".. name ..": " .. cl.get_kernel_info(kernel, name))

name = "attributes"
print("- ".. name ..": " .. cl.get_kernel_info(kernel, name))

-------------------------------------------------------------------------------
print("kernel work group info:")
name = "work group size"
print("- ".. name ..": " .. cl.get_kernel_work_group_info(kernel, device, name))

name = "preferred work group size multiple"
print("- ".. name ..": " .. cl.get_kernel_work_group_info(kernel, device, name))

name = "local mem size"
print("- ".. name ..": " .. cl.get_kernel_work_group_info(kernel, device, name))
   
name = "private mem size"
print("- ".. name ..": " .. cl.get_kernel_work_group_info(kernel, device, name))
   
name = "compile work group size"
val = cl.get_kernel_work_group_info(kernel, device, name)
print("- ".. name .. ": {" .. table.concat(val, ', ') .. "}")

name = "global work size"
ok, val = pcall(cl.get_kernel_work_group_info, kernel, device, name)
if ok then
   print("- ".. name .. ": {" .. table.concat(val, ', ') .. "}")
end

-------------------------------------------------------------------------------
print("kernel arg info:")
local function arginfo(argindex, name)
   local ok, val = pcall(cl.get_kernel_arg_info, kernel, argindex, name)
   print("  - ".. name ..": " .. (ok and val or "n.a."))
end

nargs = cl.get_kernel_info(kernel, "num args")
for i = 1, nargs do
   local name
   print("- arg " .. (i-1) ..":")
   arginfo(i-1, "type name")
   arginfo(i-1, "type qualifier")
   arginfo(i-1, "address qualifier")
   arginfo(i-1, "access qualifier")
end

-------------------------------------------------------------------------------
print()

-- Deallocate resources 
cl.release_context(context)


