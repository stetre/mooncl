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


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lua.h"
#include "lauxlib.h"

#ifndef Malloc
#define Malloc mooncl_Malloc
void *Malloc(lua_State *L, size_t size);
#define Free mooncl_Free
void Free(lua_State *L, void *ptr);
#endif

#define udata_t  mooncl_udata_t
#define udata_s  mooncl_udata_s
#define mooncl_udata_t struct mooncl_udata_s

#define udata_new mooncl_udata_new
void *udata_new(lua_State*, size_t, uint64_t, const char*);
#define udata_unref mooncl_udata_unref
int udata_unref(lua_State *L, uint64_t);
#define udata_free mooncl_udata_free
int udata_free(lua_State*, uint64_t);
#define udata_mem mooncl_udata_mem
void *udata_mem(uint64_t);
#define udata_push mooncl_udata_push
int udata_push(lua_State*, uint64_t);
#define udata_free_all mooncl_udata_free_all
void udata_free_all(lua_State *L);
#define udata_scan mooncl_udata_scan
int udata_scan(lua_State *L, const char *mt,  
            void *info, int (*func)(lua_State *L, const void *mem, const char* mt, const void *info));
#define udata_define mooncl_udata_define
int udata_define(lua_State*, const char*, const luaL_Reg*, const luaL_Reg*);
#define udata_inherit mooncl_udata_inherit
int udata_inherit(lua_State*, const char*, const char*);
#define udata_test mooncl_udata_test
void *udata_test(lua_State*, int, const char*);
#define udata_addmethods mooncl_udata_addmethods
int udata_addmethods(lua_State*, const char*, const luaL_Reg*);

#ifdef __cplusplus
}
#endif

