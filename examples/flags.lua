#!/usr/bin/env lua
-- MoonCL example: flags.lua

cl = require("mooncl")

-- Two alternative equivalent ways to produce a 'devicetypeflags' value:
code1 = cl.DEVICE_TYPE_CPU | cl.DEVICE_TYPE_GPU 
code2 = cl.devicetypeflags('cpu', 'gpu')

assert(code1 == code2) -- true
print(code1) --> 3
print(cl.devicetypeflags(code1)) --> cpu gpu
print(cl.devicetypeflags(code2)) --> cpu gpu

if (code1 & cl.DEVICE_TYPE_GPU) ~= 0 then -- NB: 0 is not false in Lua
  print("'gpu' bit is set")
else
  print("'gpu' bit is not set")
end

