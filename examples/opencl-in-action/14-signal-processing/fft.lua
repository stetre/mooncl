-- Lua port of fft_check.c
--
-- Disclaimer of the original C code:
-- /*----------------------------------------------------------------------------
-- fft.c - fast Fourier transform and its inverse (both recursively)
-- Copyright (C) 2004, Jerome R. Breitenbach.  All rights reserved.
--
-- The author gives permission to anyone to freely copy, distribute, and use
-- this file, under the following conditions:
--    - No changes are made.
--    - No direct commercial advantage is obtained.
--    - No liability is attributed to the author for any damages incurred.
--  ----------------------------------------------------------------------------*/

local TWO_PI = 2*math.pi
local cos, sin = math.cos, math.sin

local function ispow2(n)
-- Check that n is a power of 2.
   return n~=0 and (n&(~n+1))==n
end

local function zero(N)
-- Create an zero array of length N
   local x = {}
   for i=1, N do x[#x+1] = 0.0 end
   return x
end

local function fft_rec(N, offset, delta, re_x, im_x, re_X, im_X, re_XX, im_XX)
   local N2 = math.tointeger(N/2)  -- half the number of points in FFT
   local k00, k01, k10, k11 -- indices for butterflies
   local cs, sn -- cosine and sine
   local tmp0, tmp1
   if N ~= 2 then --  Perform recursive step
      -- Calculate two (N/2)-point DFT's
      fft_rec(N2, offset, 2*delta, re_x, im_x, re_XX, im_XX, re_X, im_X)
      fft_rec(N2, offset+delta, 2*delta, re_x, im_x, re_XX, im_XX, re_X, im_X)
      -- Combine the two (N/2)-point DFT's into one N-point DFT
      for k=0, N2-1 do
         k00 = offset + k*delta + 1
         k01 = k00 + N2*delta
         k10 = offset + 2*k*delta + 1
         k11 = k10 + delta
         cs = cos(TWO_PI*k/N)
         sn = sin(TWO_PI*k/N)
         tmp0 = cs * re_XX[k11] + sn * im_XX[k11]
         tmp1 = cs * im_XX[k11] - sn * re_XX[k11]
         -- print(k00, k01, k10, k11, sn, cs, tmp0, tmp1)
         re_X[k01] = re_XX[k10] - tmp0
         im_X[k01] = im_XX[k10] - tmp1
         re_X[k00] = re_XX[k10] + tmp0
         im_X[k00] = im_XX[k10] + tmp1
      end
  else  -- Perform 2-point DFT
      k00 = offset + 1
      k01 = k00 + delta
      -- print(k00, k01)
      re_X[k01] = re_x[k00] - re_x[k01]
      im_X[k01] = im_x[k00] - im_x[k01]
      re_X[k00] = re_x[k00] + re_x[k01]
      im_X[k00] = im_x[k00] + im_x[k01]
   end
end


local function fft(re_x, im_x)
-- Compute the FFT of the N-samples signal x, where N=2^n with n>=1.
-- re_x, im_x = arrays containing respectively the real and imaginary parts of the samples.
-- im_x is optional (default: all zeros, i.e. x is a real signal)
-- Returns similar arrays re_X and im_X containing the N samples of the FFT.
   local N = #re_x
   assert(ispow2(N), "the number of samples must be a power of 2")
   local im_x = im_x or zero(N)
   assert(#re_x == #im_x, "re_x and im_x must have the same length")
   local re_X, im_X = zero(N), zero(N)
   local re_XX, im_XX = zero(N), zero(N) -- scratch space
   -- Calculate FFT by a recursion
   fft_rec(N, 0, 1, re_x, im_x, re_X, im_X, re_XX, im_XX)
   return re_X, im_X
end

local function ifft(re_X, im_X)
-- Compute the IFFT of X.
   local N = #re_X
   assert(ispow2(N), "the number of samples must be a power of 2")
   local im_X = im_X or zero(N)
   assert(#re_X == #im_X, "re_X and im_X must have the same length")
   -- Calculate IFFT via reciprocity property of DFT
   local re_x, im_x = fft(re_X, im_X)
   local N2 = math.tointeger(N/2)
   re_x[1] = re_x[1]/N
   im_x[1] = im_x[1]/N
   re_x[N2+1] = re_x[N2+1]/N
   im_x[N2+1] = im_x[N2+1]/N
   local tmp0, tmp1
   for i=2, N2 do
      tmp0, tmp1 = re_x[i]/N, im_x[i]/N
      re_x[i] = re_x[N-i+2]/N
      im_x[i] = im_x[N-i+2]/N
      re_x[N-i+2] = tmp0
      im_x[N-i+2] = tmp1
   end
   return re_x, im_x
end

return {
   fft = fft,
   ifft = ifft,
   zero = zero,
}

