/* The MIT License (MIT)
 *
 * Copyright (c) 2017 Stefano Trettel
 *
 * Software repository: MoonCL, https://github.com/stetre/mooncl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "internal.h"

static lua_State *mooncl_L = NULL;

static void AtExit(void)
    {
    if(mooncl_L)
        {
        enums_free_all(mooncl_L);
        mooncl_atexit_getproc();
        mooncl_L = NULL;
        }
    }


int luaopen_mooncl(lua_State *L)
/* Lua calls this function to load the module */
    {
    mooncl_L = L;

    mooncl_utils_init(L);
    atexit(AtExit);

    lua_newtable(L); /* the cl table */
    mooncl_open_getproc(L);

    /* add cl functions: */
    mooncl_open_versions(L);
    mooncl_open_enums(L);
    mooncl_open_flags(L);
    mooncl_open_tracing(L);
    mooncl_open_datahandling(L);
    mooncl_open_platform(L);
    mooncl_open_device(L);
    mooncl_open_context(L);
    mooncl_open_queue(L);
    mooncl_open_mem(L);
    mooncl_open_buffer(L);
    mooncl_open_image(L);
    mooncl_open_pipe(L);
    mooncl_open_gl_object(L);
    mooncl_open_sampler(L);
    mooncl_open_program(L);
    mooncl_open_kernel(L);
    mooncl_open_event(L);
    mooncl_open_svm(L);
    mooncl_open_enqueue(L);
    mooncl_open_hostmem(L);

    /* Add functions implemented in Lua */
    lua_pushvalue(L, -1); lua_setglobal(L, "mooncl");
    if(luaL_dostring(L, "require('mooncl.utils')") != 0) lua_error(L);
    lua_pushnil(L);  lua_setglobal(L, "mooncl");

    return 1;
    }

