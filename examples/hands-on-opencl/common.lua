
local cl = require("mooncl")

local function printf(...) io.write(string.format(...)) end

------------------------------------------------------------------------------
-- Function to output key parameters about the input OpenCL device.
-- Pass verbose=true if you want to print info about work groups sizes.
--
-- History: Written by Tim Mattson, June 2010
--          Ported to Lua by Stefano Trettel, November 2018
local function output_device_info(device, verbose)

   printf(" \n Device is  %s ", cl.get_device_info(device, 'name'))

   local device_type = cl.get_device_info(device, 'type')
   if device_type  == cl.DEVICE_TYPE_GPU then
      printf(" GPU from ")
   elseif device_type == cl.DEVICE_TYPE_CPU then
      printf("\n CPU from ")
   else 
       printf("\n non CPU or GPU processor from ")
   end
   printf(" %s ", cl.get_device_info(device, 'vendor'))
   printf(" with a max of %d compute units \n", cl.get_device_info(device, 'max compute units'))

   if verbose then -- Optionally print information about work group sizes
      -- local max_work_itm_dims = cl.get_device_info(device, 'max work item dimensions')
      local max_loc_size = cl.get_device_info(device, 'max work item sizes')
      local max_wrkgrp_size = cl.get_device_info(device, 'max work group size')
      printf(" Work group, work item information: ")
      printf(" max loc dim = {%s}, ", table.concat(max_loc_size, ', '))
      printf(" max work group size = %d\n\n",max_wrkgrp_size)
   end
end

------------------------------------------------------------------------------
-- device_picker
-- Purpose: Provide a simple CLI to specify an OpenCL device at runtime
-- History: Method written by James Price, October 2014
--          Extracted to a common header by Tom Deakin, November 2014
--          Ported to Lua by Stefano Trettel, November 2018

local function get_device_list(arg)
   local devices = {}
   for _, platform in ipairs(cl.get_platform_ids()) do
      for _, device in ipairs(cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)) do
         table.insert(devices, device)
      end
   end
   return devices
end

local USAGE = "\nUsage: "..arg[0].." [OPTIONS]\n\n"..[[
Options:
  -h  --help               Print the message
      --list               List available devices
      --device     INDEX   Select device at INDEX
]]

local function parse_arguments()
-- Either exits or returns the device index chosen by the user
   for i = 1, #arg do
      if arg[i] == "--list" then
         local devices = get_device_list()
         if #devices == 0 then
            printf("No devices found.\n")
         else
            printf("\nDevices:\n")
            for i, device in ipairs(devices) do
               printf("%2d: %s\n", i, cl.get_device_info(device, "name"))
            end
            printf("\n")
         end
         os.exit(true)
      elseif arg[i] == "--device" then
         local device_index = math.tointeger(arg[i+1])
         if device_index then return device_index end
         printf("Invalid device index\n")
         os.exit(true)
      elseif arg[i] == "--help" or arg[i] == "-h" then
         print(USAGE)
         os.exit(true)
      else
         printf("unknown option '%s'\n", arg[i])
         print(USAGE)
         os.exit(true)
      end
   end
   return 1 -- default to first device
end

------------------------------------------------------------------------------
return {
   output_device_info = output_device_info,
   get_device_list = get_device_list,
   parse_arguments = parse_arguments,
}

