#!/usr/bin/env lua
-- Checking floating-point supported features 

local cl = require('mooncl')

--cl.trace_objects(true)

-- Select a device 
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]

local flags = cl.get_device_info(device, 'single fp config')
-- alt: local flags = cl.get_device_info(device, cl.DEVICE_SINGLE_FP_CONFIG)

print("fp flags: " .. flags.." ("..table.concat({cl.fpflags(flags)}, ', ')..")")

print("Float Processing Features:")
if (flags & cl.FP_INF_NAN) ~= 0 then
   print("INF and NaN values supported.")
end
if (flags & cl.FP_DENORM) ~= 0 then
   print("Denormalized numbers supported.")
end
if (flags & cl.FP_ROUND_TO_NEAREST) ~= 0 then
   print("Round To Nearest Even mode supported.")
end
if (flags & cl.FP_ROUND_TO_INF) ~= 0 then
   print("Round To Infinity mode supported.")
end
if (flags & cl.FP_ROUND_TO_ZERO) ~= 0 then
   print("Round To Zero mode supported.")
end
if (flags & cl.FP_FMA) ~= 0 then
   print("Floating-point multiply-and-add operation supported.")
end
if (flags & cl.FP_SOFT_FLOAT) ~= 0 then
   print("Basic floating-point processing performed in software.")
end

