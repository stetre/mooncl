#!/usr/bin/env lua
-- Display Device Information
-- Script to print out some information about the OpenCL devices
-- and platforms available on your system.
--
-- History: C++ version written by Tom Deakin, 2012
--          Ported to C by Tom Deakin, July 2013
--          Updated by Tom Deakin, October 2014
--          Ported to Lua by Stefano Trettel, November 2018

local cl = require("mooncl")

local function printf(...) io.write(string.format(...)) end

-- Get the list of available platforms
local platform = cl.get_platform_ids()

if #platform == 0 then
   printf("Found 0 platforms!\n")
   os.exit(true)
end

printf("\nNumber of OpenCL platforms: %d\n", #platform)
printf("\n-------------------------\n")

-- Investigate each platform
for i=1, #platform do
   -- Print out the platform name
   printf("Platform: %s\n", cl.get_platform_info(platform[i], 'name'))
   -- alt: printf("Platform: %s\n", cl.get_platform_info(platform[i], cl.PLATFORM_NAME))

   -- Print out the platform vendor
   printf("Vendor: %s\n", cl.get_platform_info(platform[i], 'vendor'))

   -- Print out the platform OpenCL version
   printf("Version: %s\n", cl.get_platform_info(platform[i], 'version'))

   -- Get the devices in the platform
   local device = cl.get_device_ids(platform[i], cl.DEVICE_TYPE_ALL)
   printf("Number of devices: %d\n", #device)

   -- Investigate each device
   for j = 1, #device do
      printf("\t-------------------------\n")

      -- Get device name
      printf("\tName: %s\n", cl.get_device_info(device[j], 'name'))

      -- Get device OpenCL version
      printf("\tVersion: %s\n", cl.get_device_info(device[j], 'opencl c version'))

      -- Get Max. Compute units
      printf("\tMax. Compute Units: %d\n",
         cl.get_device_info(device[j], 'max compute units'))

      -- Get local memory size
      printf("\tLocal Memory Size: %.0f KB\n",
         cl.get_device_info(device[j], 'local mem size')/1024)

      -- Get global memory size
      printf("\tGlobal Memory Size: %.0f MB\n",
         cl.get_device_info(device[j], 'global mem size')/(1024*1024))

      -- Get maximum buffer alloc. size
      printf("\tMax Alloc Size: %.0f MB\n",
         cl.get_device_info(device[j], 'max mem alloc size')/(1024*1024))

      -- Get work-group size information
      printf("\tMax Work-group Total Size: %d\n", 
         cl.get_device_info(device[j], 'max work group size'))

      -- Find the maximum dimensions of the work-groups
      printf("\tMax Work-item Dimensions: %d\n",
         cl.get_device_info(device[j], 'max work item dimensions'))

      -- Get the max. size for each dimension of the work-groups
      local size = cl.get_device_info(device[j], 'max work item sizes')
            printf("\tMax Work-item Sizes: { "..table.concat(size, ', ').." }\n")
            printf("\t-------------------------\n");
        printf("-------------------------\n");

   end
end

