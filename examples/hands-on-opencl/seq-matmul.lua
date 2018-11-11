#!/usr/bin/env lua
----------------------------------------------------------------------------------
-- Run sequential version of matmul on the host
----------------------------------------------------------------------------------

local cl = require("mooncl")
local mat = require("matrix_lib")
local function printf(...) io.write(string.format(...)) end

--  Constants
local N     = 1024    -- Order of the square matrices A, B, and C
local AVAL  = 3.0     -- A elements are constant and equal to AVAL
local BVAL  = 5.0     -- B elements are constant and equal to BVAL
local TOL   = 0.001   -- tolerance used in floating point comparisons
local COUNT = 1       -- number of times to do each multiplication

local A = mat(N, AVAL)
local B = mat(N, BVAL)

printf("\n===== Sequential (Lua), matrix mult (dot prod), order %d on host CPU ======\n",N)
for i = 1, COUNT do
   local t = cl.now()
   local C = A*B
   t = cl.since(t)
   C:results(AVAL, BVAL, TOL, t)
end

