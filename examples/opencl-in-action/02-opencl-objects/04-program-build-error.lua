#!/usr/bin/env lua
-- Example of a failure when building a program.

local cl = require('mooncl')

local filenames = { "good.cl", "bad.cl" }
local options = "-cl-finite-math-only -cl-no-signed-zeros"

-- Access the first device on the first platform 
local platform = cl.get_platform_ids()[1]
local device = cl.get_device_ids(platform, cl.DEVICE_TYPE_ALL)[1]

-- Create the context 
local context = cl.create_context(platform, {device})

-- Collect the program sources from the list of files, concacatenate
-- them into a single soorce string, and create a program from it:
local program_sources = {}
for i, filename in ipairs(filenames) do
   local f = assert(io.open(filename, 'r'))
   program_sources[i] = f:read('a')
   f:close()
end
local source = table.concat(program_sources, " ")
local program = cl.create_program_with_source(context, source)

-- Build the program. We expect this to fail, because the good() function
-- is defined in both the good.cl and bad.cl source files:
print("\n   ***   The following failure is intentional   *** \n")
cl.build_program(program, {device}, options)

cl.release_program(program)
cl.release_context(context)

