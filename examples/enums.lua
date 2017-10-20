#!/usr/bin/env lua
-- MoonCL example: enums.lua

cl = require("mooncl")

-- Alternative and equivalent ways to assign values of the 'filtermode' enum type
-- (whose hint is cl.FILTER_XXX):
op1 = cl.FILTER_NEAREST 
op2 = 'nearest' -- XXX=NEAREST
op3 = cl.FILTER_LINEAR
op4 = 'linear' -- XXX=LINEAR

print(op1) --> 'nearest'
print(op2) --> 'nearest'
print(op3) --> 'linear'
print(op4) --> 'linear'
print(op1 == 'nearest') --> true
print(op3 == op4) --> true

-- List the literals admitted by the 'filtermode' enum type:
ops = cl.enum('filtermode')
print(table.concat(ops, ', ')) --> 'nearest', 'linear'

