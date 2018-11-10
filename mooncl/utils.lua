-- The MIT License (MIT)
--
-- Copyright (c) 2017 Stefano Trettel
--
-- Software repository: MoonCL, https://github.com/stetre/mooncl
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
-- 

-- *********************************************************************
-- DO NOT require() THIS MODULE (it is loaded automatically by MoonCL)
-- *********************************************************************

local cl = mooncl -- require("mooncl")

function cl.create_program_with_sourcefile(context, filename)
   local f, errmsg = io.open(filename)
   if not f then error(errmsg, 2) end
   local source = f:read('a')
   f:close()
   local ok, program = pcall(cl.create_program_with_source, context, source)
   if not ok then error(program, 2) end
   return program
end

function cl.build_program(program, devices, options)
   local ok, errmsg = cl.build_program_(program, devices, options)
   if ok then return end
   local t = { errmsg }
   if errmsg == "build program failure" and devices then
      for i, dev in ipairs(devices) do
         t[#t + 1] = "device-".. i .." log:"
         t[#t + 1] = cl.get_program_build_info(program, dev, 'log')
      end
   end
   error(table.concat(t, '\n'), 2)
end

function cl.make_program_with_source(context, source, devices, options)
   local ok, program = pcall(cl.create_program_with_source, context, source)
   if not ok then error(program, 2) end
   local ok, errmsg = pcall(cl.build_program, program, devices, options)
   if not ok then error(errmsg, 2) end
   return program
end

function cl.make_program_with_sourcefile(context, filename, devices, options)
   local ok, program = pcall(cl.create_program_with_sourcefile, context, filename)
   if not ok then error(program, 2) end
   local ok, errmsg = pcall(cl.build_program, program, devices, options)
   if not ok then error(errmsg, 2) end
   return program
end

function cl.release_events(events)
   for _, ev in ipairs(events) do cl.release_event(ev) end
end

function cl.is_extension_supported(where, name)
   local extensions = cl.type(where) == "device"
      and cl.get_device_info(where, "extensions")
      or cl.get_platform_info(where, "extensions")
   return string.find(extensions, name) and true or false
end
