## MoonCL: Lua bindings for OpenCL

MoonCL is a Lua binding library for the [Khronos OpenCL&#8482; API](https://www.khronos.org/opencl).

It runs on GNU/Linux <!-- and on Windows (MSYS2/MinGW) --> and requires 
[Lua](http://www.lua.org/) (>=5.3) and OpenCL (>= 1.2).


_Author:_ _[Stefano Trettel](https://www.linkedin.com/in/stetre)_

[![Lua logo](./doc/powered-by-lua.gif)](http://www.lua.org/)

#### License

MIT/X11 license (same as Lua). See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://stetre.github.io/mooncl/doc/index.html).

#### Getting and installing

Setup the build environment as described [here](https://github.com/stetre/moonlibs), then:

```sh
$ git clone https://github.com/stetre/mooncl
$ cd mooncl
mooncl$ make
mooncl$ sudo make install
```

To use MoonCL, you'll also need the [OpenCL ICD loader](https://github.com/KhronosGroup/OpenCL-ICD-Loader)
and at least one 
[OpenCL capable device](https://www.khronos.org/conformance/adopters/conformant-products#opencl) with
[updated drivers](https://www.howtogeek.com/242045/how-to-get-the-latest-nvidia-amd-or-intel-graphics-drivers-on-ubuntu/).
 
On Ubuntu, you can install the OpenCL ICD loader with:

```sh
$ sudo apt install ocl-icd-opencl-dev
```

#### Example

The example below performs a simple addition using the first available GPU.

Other examples can be found in the **examples/** directory contained in the release package.

```lua
-- MoonCL example: hello.lua
cl = require("mooncl")

-- Select platform and device, create context and command_queue
platform = cl.get_platform_ids()[1]
device = cl.get_device_ids(platform, cl.DEVICE_TYPE_GPU)[1]
context = cl.create_context(platform, {device})
queue = cl.create_command_queue(context, device)

-- Create program and build it
program = cl.create_program_with_source(context,[[
   kernel void myadd(uint a, uint b, global uint *result) {
      *result = a + b;
   }
]]) 

cl.build_program(program, {device})
 
-- Create kernel 
kernel = cl.create_kernel(program, "myadd")

-- Create a buffer to hold the result
result_buffer = cl.create_buffer(context, cl.MEM_WRITE_ONLY, cl.sizeof('uint'))
         
-- Set kernel arguments 
a, b = 20, 30
cl.set_kernel_arg(kernel, 0, 'uint', a)
cl.set_kernel_arg(kernel, 1, 'uint', b)
cl.set_kernel_arg(kernel, 2, result_buffer)

-- Enqueue kernel for execution
cl.enqueue_task(queue, kernel)

-- Read and print the result 
mem = cl.malloc(cl.sizeof('uint'))
cl.enqueue_read_buffer(queue, result_buffer, true, 0, mem:size(), mem:ptr())
result = mem:read(0, nil, 'uint')
   
-- Print result, and check it
print("".. a .. " + " .. b .. " = " .. result[1])
assert(result[1] == a + b)

-- Deallocate resources 
cl.release_context(context)

```

The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ lua hello.lua
```

#### See also

* [MoonLibs - Graphics and Audio Lua Libraries](https://github.com/stetre/moonlibs).
