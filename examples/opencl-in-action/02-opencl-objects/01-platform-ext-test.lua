#!/usr/bin/env lua
-- Search for a platform that supports a given extension.

local cl = require('mooncl')

local required_extension = "cl_khr_icd" -- extension to look for

local platforms = cl.get_platform_ids() -- all available platforms
local found -- index of platform supporting required_extension
for i, platform in ipairs(platforms) do
   local ext_data = cl.get_platform_info(platform, 'extensions')
   print("\nPlatform "..i.." supports extensions: ".. ext_data)
   -- Look for required_extension
   if not found and string.find(ext_data, required_extension) then
		found = i
		break
	end
end

local s = found and ("\nPlatform "..found) or "No platform"
print("\n"..s.." supports the "..required_extension.." extension\n")

