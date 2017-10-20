#!/usr/bin/env lua
-- MoonCL example: versions.lua
-- Detects and prints the versions supported by the loader and by 
-- the available platforms and devices

cl = require("mooncl")

print()
print("MoonCL version: ".. cl._VERSION)

print()
print("Supported versions:")
--print("     ".. table.concat(cl.CL_VERSIONS, ', ')) 
print("- CL_VERSION_1_0: " .. (cl.CL_VERSION_1_0 and "yes" or "no"))
print("- CL_VERSION_1_1: " .. (cl.CL_VERSION_1_1 and "yes" or "no"))
print("- CL_VERSION_1_2: " .. (cl.CL_VERSION_1_2 and "yes" or "no"))
print("- CL_VERSION_2_0: " .. (cl.CL_VERSION_2_0 and "yes" or "no"))
print("- CL_VERSION_2_1: " .. (cl.CL_VERSION_2_1 and "yes" or "no"))
print("- CL_VERSION_2_2: " .. (cl.CL_VERSION_2_2 and "yes" or "no"))

print()
print("Versions supported by the available platforms and devices:")
platforms = cl.get_platform_ids()

for i, plat in ipairs(platforms) do
   print("- platform no. "..i..": " 
         .. cl.get_platform_info(plat, "version") .. 
         " (".. cl.get_platform_info(plat, "profile") ..")")
   local devices = cl.get_device_ids(plat, cl.devicetypeflags('all'))
   for j, dev in ipairs(devices) do
      print("  - device no. "..j..": " ..
            cl.get_device_info(dev, "version") ..
            " (" .. cl.get_device_info(dev, "profile") .. ")" ..
            " - " .. cl.get_device_info(dev, "opencl c version"))
   end
end

print()

