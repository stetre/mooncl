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
    
static int AddVersions(lua_State *L)
    {
    int i = 0;
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, "MoonCL "MOONCL_VERSION);
    lua_settable(L, -3);

    /* supported versions --------------------------- */
    lua_pushstring(L, "CL_VERSIONS");
    lua_newtable(L);
#define ADD(ver) do {               \
    lua_pushstring(L, ""#ver);      \
    lua_rawseti(L, -2, ++i);        \
    lua_pushboolean(L, 1);          \
    lua_setfield(L, -4, ""#ver);    \
} while(0)
    ADD(CL_VERSION_1_0);
    ADD(CL_VERSION_1_1);
    ADD(CL_VERSION_1_2);
    ADD(CL_VERSION_2_0);
    ADD(CL_VERSION_2_1);
    ADD(CL_VERSION_2_2);
    lua_settable(L, -3);
#undef ADD
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { NULL, NULL } /* sentinel */
    };

void mooncl_open_versions(lua_State *L)
    {
    AddVersions(L);
    luaL_setfuncs(L, Functions, 0);
    }

