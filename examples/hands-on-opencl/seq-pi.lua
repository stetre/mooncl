#!/usr/bin/env lua

-- This program will numerically compute the integral of 4/(1+x*x)
-- from 0 to 1.  The value of this integral is pi - which
-- is great since it gives us an easy way to check the answer.
-- This is the original sequential program.
--
-- History: Written by Tim Mattson, 11/99.
--          Ported to Lua by Stefano Trettel, 11/18
--

local cl = require("mooncl") -- only for time functions
local num_steps = 100000000

local pi, sum = 0, 0
local x
local step = 1.0/num_steps

local t = cl.now()
for i=0, num_steps-1 do
   x = (i-0.5)*step
   sum = sum + 4.0/(1.0+x*x)
end
pi = step * sum
t = cl.since(t)
print(string.format("\n pi with %d steps is %f in %f seconds", num_steps, pi, t))

