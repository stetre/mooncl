-- Matrix Market I/O library for Lua
-- -----------------------------------------------------------------------------------
-- See http://math.nist.gov/MatrixMarket for details on the MM format.
-- 
-- Internally, a matrix A is represented as a table with the following entries:
-- matrix.sparse = true | false
-- matrix.datatype = 'real' | 'integer' | 'complex' | 'pattern'
-- matrix.symmetry = 'general' | 'hermitian' | 'symmetric' | 'skew-symmetric'
-- matrix.data = array of elements, where each element is a table 
--    element = { i, j, A(i,j) }                   if datatype = 'real' or 'integer'
--    element = { i, j, re(A(i,j)), im(A(i,j)) }   if datatype = 'complex'
--    element = { i, j }                           if datatype = 'pattern'
--
-- The indices i and j are 1-based.
--
-- If symmetry = 'hermitian', then re(A(j,i))=re(A(i,j)) and im(A(j,i))=-im(A(i,j))
-- If symmetry = 'symmetric', then A(j,i) = A(i,j)
-- If symmetry = 'skew-symmetric', then A(j,i) = -A(i,j)

local function emptyline(line)
   return line and string.match(line, "^%s*$")
end
local function emptyline_or_comment(line)
   return line and string.match(line, "^%s*$") or string.match(line, "^%%.*")
end

local valid_density  = {coordinate=true, array=true}
local valid_datatype = {real=true, complex=true, pattern=true, integer=true}
local valid_symmetry = {general=true, hermitian=true, symmetric=true, ['skew-symmetric']=true}

----------------------------------------------------------------------------------
local function mmio_read(filename)
   local f = assert(io.open(filename, 'r'))
   local function err(what) f:close() return nil, what end
   local line
   repeat line = f:read('l') until not emptyline(line)
   if not line then return err("premature end of file") end
   -- Read banner: %%MatrixMarket density datatype symmetry
   local density, datatype, symmetry = 
      string.match(line, "^%s*%%%%MatrixMarket%s+matrix%s+(%a+)%s+(%a+)%s+([%a%-]+)")
   if not density then return err("invalid banner") end
   if not valid_density[density] or not valid_datatype[datatype] or not valid_symmetry[symmetry] then
      return err("invalid banner")
   end
   repeat line = f:read('l') until not emptyline_or_comment(line)
   if not line then return err("premature end of file") end
   -- Read size
   local M, N, L  -- MxN matrix with L nonzeros following in the file
   if density == 'coordinate' then
      M, N, L = string.match(line, "^%s*(%d+)%s+(%d+)%s+(%d+)%s*$")
   else -- if density == 'array' then
      M, N = string.match(line, "^%s*(%d+)%s+(%d+)%s*$")
   end
   -- @@ check M, N, L  
   repeat line = f:read('l') until not emptyline(line)
   if not line then return err("premature end of file") end
   local data = {}
   if datatype == 'pattern' then
      while line do
         local i, j = string.match(line, "^%s*(%d+)%s+(%d+)")
         data[#data+1] = { tonumber(i), tonumber(j) }
         repeat line = f:read('l') until not emptyline(line)
      end
   elseif datatype == 'complex' then
      while line do
         local i, j, re, im = string.match(line, "^%s*(%d+)%s+(%d+)%s+([%+%-%de%.]+)%s+([%+%-%de%.]+)")
         data[#data+1] = { tonumber(i), tonumber(j), tonumber(re), tonumber(im) }
         repeat line = f:read('l') until not emptyline(line)
      end
   else -- if datatype == 'real' or datatype == 'integer' then
      while line do
         local i, j, val = string.match(line, "^%s*(%d+)%s+(%d+)%s+([%+%-%de%.]+)")
         data[#data+1] = { tonumber(i), tonumber(j), tonumber(val) }
         repeat line = f:read('l') until not emptyline(line)
      end
   end
   f:close()
   return {
      sparse = (density=="coordinate"),
      datatype = datatype,
      symmetry = symmetry,
      M = M,
      N = N,
      L = L or nil,
      data = data
   }
end

----------------------------------------------------------------------------------
local function mmio_write(matrix, filename)
   local f = assert(io.open(filename, 'w'))
   local fmt = string.format
   local function err(what) f:close() return nil, what end
   local sparse, datatype, symmetry = matrix.sparse, matrix.datatype, matrix.symmetry
   local M, N, data = matrix.M, matrix.N, matrix.data
   if not valid_datatype[datatype] then return err("invalid datatype") end
   if not valid_symmetry[symmetry] then return err("invalid symmetry") end
   if not math.tointeger(M) or not math.tointeger(N) then return err("invalid size") end
   if sparse then
      f:write(fmt("%%%%MatrixMarket matrix coordinate %s %s\n", datatype, symmetry))
      f:write(fmt("%d %d %d\n", M, N, #data))
   else
      f:write(fmt("%%MatrixMarket matrix array %s %s\n", datatype, symmetry))
      f:write(fmt("%d %d\n", M, N))
   end
   -- Sort by column:
   table.sort(data, function(a, b) return a[2]==b[2] and a[1]<b[1] or a[2]<b[2] end)
   -- Write data:
   if datatype == 'pattern' then
      for _, v in ipairs(data) do f:write(fmt("%d %d\n", v[1], v[2])) end
   elseif datatype == 'complex' then
      for _, v in ipairs(data) do f:write(fmt("%d %d %20.16g %20.16g\n", v[1], v[2], v[3], v[4])) end
   elseif datatype == 'real' then
      for _, v in ipairs(data) do f:write(fmt("%d %d %20.16g\n", v[1], v[2], v[3])) end
   else -- if datatype == 'integer' then
      for _, v in ipairs(data) do f:write(fmt("%d %d %20d\n", v[1], v[2], v[3])) end
   end
   f:close()
end

----------------------------------------------------------------------------------
local function mmio_fulldata(matrix, zerobased, order)
-- Given a matrix in the internal representation (as returned by mmio_read()),
-- returns a table with the full non-zero data elements (e.g., if the matrix is
-- symmetric, the returned table contains also the elements of the lower triangle).
-- zerobased = true: convert the indices from 1-based to 0-based
--             false: don't.
--     order = 'row': order the elements in row-major order (default) 
--             'column': order the elements in column-major order
   local ofs = zerobased and 1 or 0
   local sparse, datatype, symmetry = matrix.sparse, matrix.datatype, matrix.symmetry
   local data = {}
   if symmetry == 'general' then
      for i=1,#matrix.data do
         local i, j, a, b = table.unpack(matrix.data[i])
         data[#data+1] = { i-ofs, j-ofs, a, b }
      end
   elseif symmetry == 'symmetric' then
      for i=1,#matrix.data do
         local i, j, a, b = table.unpack(matrix.data[i])
         data[#data+1] = { i-ofs, j-ofs, a, b }
         if i~=j then data[#data+1] = { j-ofs, i-ofs, a, b } end
      end
   elseif symmetry == 'skew-symmetric' then
      for i=1,#matrix.data do
         local i, j, a, b = table.unpack(matrix.data[i])
         data[#data+1] = { i-ofs, j-ofs, a, b }
         if i~=j then data[#data+1] = { j-ofs, i-ofs, a and -a, b and -b } end
      end
   elseif symmetry == 'hermitian' then
      for i=1,#matrix.data do
         local i, j, a, b = table.unpack(matrix.data[i])
         data[#data+1] = { i-ofs, j-ofs, a, b }
         if i~=j then data[#data+1] = { j-ofs, i-ofs, a, b and -b } end
      end
   end
   local order = order or 'row'
   if order == 'row' then
      table.sort(data, function(a, b) return a[1]==b[1] and a[2]<b[2] or a[1]<b[1] end)
   elseif order == 'column' then
      table.sort(data, function(a, b) return a[2]==b[2] and a[1]<b[1] or a[2]<b[2] end)
   else
      error('invalid order')
   end
   return data
end

return {
   read = mmio_read,
   write = mmio_write,
   fulldata = mmio_fulldata,  
}

