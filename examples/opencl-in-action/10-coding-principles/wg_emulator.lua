#!/usr/bin/env lua
-- Emulator for the reduction_scalar kernel in reduction.cl.
-- Prints the operations done by a work-group, in the order they are executed.

local group_size = 32

-- Indices for the inner loop (powers of 2 from group_size/2 down to 1)
local indices = {}
local i = group_size>>1
while i > 0 do indices[#indices+1]=i i=i>>1 end
-- print("indices = "..table.concat(indices, ', '))

local stage = {}
for lid = 0, group_size-1 do
   -- Each iteration is a work-item. In the real kernel these iterations are
   -- potentially (and hopefully) done in parallel.
   local n = 1 -- stage number
   for _, i in ipairs(indices) do -- in C: for(i=group_size/2; i>0; i>>1)
      if not stage[n] then stage[n] = {} end
      -- print(lid, i, lid+i,lid < i)
      if lid < i then
         table.insert(stage[n], string.format("  partial_sums[%d] += partial_sums[%d]", lid, lid+i))
      end
      -- In the real kernel there is a barrier here, so the work-item stops
      -- here until all the other work-items in the same group arrive here.
      n = n + 1 -- go to the subsequent stage
   end
end

for n, s in ipairs(stage) do
   print("stage "..n.. " (i="..indices[n]..")")
   print("  parallel operations:")
   for _, op in ipairs(s) do print(op) end
   print("  barrier")
end

