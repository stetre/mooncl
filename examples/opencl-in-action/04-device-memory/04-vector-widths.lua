#!/usr/bin/env lua
-- Checking the preferred vector widths
local cl = require('mooncl')

local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]

print("Preferred vector width in chars: "..cl.get_device_info(device, 'preferred vector width char'))
print("Preferred vector width in shorts: "..cl.get_device_info(device, 'preferred vector width short'))
print("Preferred vector width in ints: "..cl.get_device_info(device, 'preferred vector width int'))
print("Preferred vector width in longs: "..cl.get_device_info(device, 'preferred vector width long'))
print("Preferred vector width in halfs: "..cl.get_device_info(device, 'preferred vector width half'))
print("Preferred vector width in floats: "..cl.get_device_info(device, 'preferred vector width float'))
print("Preferred vector width in doubles: "..cl.get_device_info(device, 'preferred vector width double'))
 
