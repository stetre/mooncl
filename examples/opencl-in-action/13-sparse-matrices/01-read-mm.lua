#!/usr/bin/env lua
-- Read a Matrix Market file and print its contents

local mmio = require("mmio")

local function printf(...) io.write(string.format(...)) end

local filename = arg[1] or "bcsstk05.mtx"
local matrix = assert(mmio.read(filename))

printf("\nThe MM file '%s' contains a %s, %s, %dx%d matrix of %s numbers.\n", 
   filename,
   matrix.sparse and "sparse" or "dense", 
   matrix.symmetry,
   matrix.M, matrix.N,
   matrix.datatype)

if matrix.L then
   printf("It has %d non-zero entries.\n", matrix.L)
end

-- Add the missing non-zero elements, convert indices to 0-based, and order
-- elements in row-major order
local data = mmio.fulldata(matrix, true, 'row')
local datatype = matrix.datatype
local n = #data
n = math.min(n, 10) -- limit output to 10 values (comment out if you want them all)

printf("The non-zero entries of the matrix are:\n")
if datatype == "real" or datatype == "integer" then
   for i = 1, n do
      local v = data[i]
      printf("m(%d, %d) = %.16g\n", v[1], v[2], v[3])
   end
elseif datatype == "complex" then
   for i = 1, n do
      local v = data[i]
      printf("m(%d, %d) = %.16g + i %.16g\n", v[1], v[2], v[3], v[4])
   end
elseif datatype == "pattern" then
   for i = 1, n do
      local v = matrix.data[i]
      printf("m(%d, %d)\n", v[1], v[2])
   end
end
if n ~= #data then printf("...\n") end

