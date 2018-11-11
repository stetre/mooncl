-- Matrix library for the multiplication driver
--
-- Purpose: This is a simple set of functions to manipulate
--          matrices used with the multiplcation driver.
--
-- History: Written by Tim Mattson, August 2010
--          Modified by Simon McIntosh-Smith, September 2011
--          Modified by Tom Deakin and Simon McIntosh-Smith, October 2012
--          Ported to C by Tom Deakin, 2013
--          Ported to Lua by Stefano Trettel, November 2018
--------------------------------------------------------------------------------

local function printf(...) io.write(string.format(...)) end

local M = {}

local function new(N, val)
-- Creates an NxN matrix with its elements initialized to val (val defaults to 0)
   local N, val = N, val or 0
   local A = {}
   for i = 1, N do
      A[i] = {}
      for j = 1, N do
         A[i][j] = val
      end
   end
   return setmetatable(A,
      { __index = M,
        __tostring = function(A)
            local s = {}
            for i = 1, N do 
               s[i] = table.concat(A[i], ', ')
            end
            return table.concat(s, "\n")
         end,
         __mul = function(A, B) return M.mul(A, B) end,
      })
end

function M.init(A, val)
-- A:init()      inits all the elements to 0
-- A:init(val)   inits all the elements to val
-- A:init({val}) inits the elements with the values in {val} (row-major order)
   local N = #A
   if type(val) == "table" then
      local k = 1
      for i = 1, N do
         for j = 1, N do
            A[i][j] = val[k] or 0
            k = k + 1
         end
      end
   else
      local val = val or 0
      for i = 1, N do
         for j = 1, N do
            A[i][j] = val
         end
      end
   end
end

function M.transpose(A)
--  Returns the transpose
   local N = #A
   local tmat = new(N)
   for i = 1, N do
      for j = 1, N do
         tmat[i][j] = A[j][i]
      end
   end
end

function M.mul(A, B)
--  Function to compute the matrix product (sequential algorithm, dot prod)
   local N = #A
   local C = new(N)
   for i = 1, N do
      for j = 1, N do
         local tmp = 0
         for k = 1, N do
            tmp = tmp + A[i][k]*B[k][j]
         end
         C[i][j] = tmp
      end
   end
   return C
end

function M.compute_error(C, aval, bval)
--  Function to compute errors of the product matrix C=A*B where
--  A is a NxN matrix with all elements = aval and
--  B is a NxN matrix with all elements = bval.
   local N = #C
   local cval = N*aval*bval 
   local errsq, err = 0, 0
   for i = 1, N do
      for j = 1, N do
         err = C[i][j] - cval
         errsq = errsq + err*err
      end
   end
   return errsq
end

function M.results(C, aval, bval, tolerance, run_time)
--  Function to analyze and output results
   local N = #C
   local mflops = 2.0*N^3/(1e6*run_time)
   printf(" %.2f seconds at %.1f MFLOPS \n", run_time, mflops)
   local errsq = C:compute_error(aval, bval)
   if errsq > tolerance then
      printf("\n Errors in multiplication: %f\n",errsq)
   end
end

return new
