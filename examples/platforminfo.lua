#!/usr/bin/env lua
-- MoonCL example: platforminfo.lua
--
-- Lists info about available platforms.
--
cl = require("mooncl")

ok, platforms = pcall(cl.get_platform_ids)
if not ok then 
   print(platforms) -- error message
   os.exit()
end

print()
print("Found ".. #platforms .. " platform" .. (#platforms > 1 and "s." or "."))

for i, p in ipairs(platforms) do
   print()
   print("platform no. "..i..":")
   print("- name: ".. cl.get_platform_info(p, "name"))
   print("- vendor: ".. cl.get_platform_info(p, "vendor"))
   print("- version: ".. cl.get_platform_info(p, "version"))
   print("- profile: ".. cl.get_platform_info(p, "profile"))
   local extensions =  cl.get_platform_info(p, "extensions")
   print("- extensions: ")
   for e in string.gmatch(extensions, "[%w_]+") do 
      print("   "..e) 
   end
   
   local devices = cl.get_device_ids(p, cl.DEVICE_TYPE_ALL)
   print("- devices: ")
   for k, d in ipairs(devices) do
      print("   "..k..": " .. cl.get_device_info(d, "name") ..
         " (" .. cl.get_device_info(d, "version") .. 
         " - " .. cl.get_device_info(d, "profile") .. ")")
   end
end

print()
