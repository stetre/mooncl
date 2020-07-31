#!/usr/bin/env lua
-- Print information about a device.

local cl = require('mooncl')

-- Get the first platform and its first device of any type:
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]

-- Get the device name, address size, and the list of extensions it supports:
local name = cl.get_device_info(device, 'name')
local bits = cl.get_device_info(device, 'address bits')
local extensions = cl.get_device_info(device, 'extensions')
-- Print them out
print("NAME: "..name)
print("ADDRESS_WIDTH: "..bits)
print("EXTENSIONS: "..extensions)

-- Having retrieved the extensions, we can check if a particular extension is supported:
local required_extension = "cl_khr_icd"
local supported = (string.find(extensions, required_extension) ~= nil)
print("The '"..required_extension.."' extension is " ..(supported and "" or "not ").."supported")

-- Alternatively, MoonCL provides the cl.is_extension_supported() utility that retrieves
-- the extensions info for us, and checks the support for the given extension:
local supported = cl.is_extension_supported(platform, required_extension)
print("The '"..required_extension.."' extension is " ..(supported and "" or "not ").."supported by the platform")
local supported = cl.is_extension_supported(device, required_extension)
print("The '"..required_extension.."' extension is " ..(supported and "" or "not ").."supported by the device")

