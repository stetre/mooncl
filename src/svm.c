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

static int freesvm(lua_State *L, ud_t *ud)
    {
    cl_svm svm = (cl_svm)ud->handle;
    cl_context context = ud->context;
    int dont_free = IsSvmDontFree(ud);
    if(!freeuserdata(L, ud, "svm")) return 0;
    CheckPfn_2_0(L, SVMFree); //if we are at this point, SVMAlloc != 0 so SVMFree should also...
    cl.SVMFree(context, svm->ptr);
    if(!dont_free)
        Free(L, svm);
    return 0;
    }

static int newsvm(lua_State *L, cl_context context, cl_svm svm)
    {
    ud_t *ud;
    ud = newuserdata(L, svm, SVM_MT, "svm");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freesvm;
    return 1;
    }

static int Create(lua_State *L)
    {
    cl_svm svm;
    cl_context context = checkcontext(L, 1, NULL);
    cl_svm_mem_flags flags = checkflags(L, 2);
    size_t size = luaL_checkinteger(L, 3);
    cl_uint alignment = luaL_checkinteger(L, 4);

    CheckPfn_2_0(L, SVMAlloc);
    svm = (cl_svm)Malloc(L, sizeof(svm_t));

    svm->ptr = cl.SVMAlloc(context, flags, size, alignment);

    if(svm->ptr == NULL)
        {
        Free(L, svm);
        return luaL_error(L, "cl.SVMAlloc failed to allocate buffer");
        }
    
    svm->flags = flags;
    svm->size = size;
    svm->alignment = alignment;
    newsvm(L, context, svm);
    return 1;
    }

static int Ptr(lua_State *L)
    {
    cl_svm svm = checksvm(L, 1, NULL);
    lua_pushlightuserdata(L, svm->ptr);
    return 1;
    }

static int Size(lua_State *L)
    {
    cl_svm svm = checksvm(L, 1, NULL);
    lua_pushinteger(L, svm->size);
    return 1;
    }

static int Alignment(lua_State *L)
    {
    cl_svm svm = checksvm(L, 1, NULL);
    lua_pushinteger(L, svm->alignment);
    return 1;
    }

static int Flags(lua_State *L)
    {
    cl_svm svm = checksvm(L, 1, NULL);
    lua_pushinteger(L, svm->flags);
    return 1;
    }

//RAW_FUNC(svm)
TYPE_FUNC(svm)
CONTEXT_FUNC(svm)
PARENT_FUNC(svm)
DELETE_FUNC(svm)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Ptr },
        { "type", Type },
        { "context", Context },
        { "parent", Parent },
        { "delete", Delete },
        { "ptr", Ptr },
        { "size", Size },
        { "alignment", Alignment },
        { "flags", Flags },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };


static const struct luaL_Reg Functions[] = 
    {
        { "svm_alloc", Create },
        { "svm_free",  Delete },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_svm(lua_State *L)
    {
    udata_define(L, SVM_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

