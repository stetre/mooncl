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

GET_INFO_ENUM(GetMemType, MemObject, mem, mem, pushmemobjecttype)
GET_INFO_FLAGS(GetFlags, MemObject, mem, mem)
GET_INFO_SIZE(GetSize, MemObject, mem, mem)
GET_INFO_UINT(GetUint, MemObject, mem, mem)
GET_INFO_OBJECT(GetContext, MemObject, mem, mem, context)
GET_INFO_BOOLEAN(GetBoolean, MemObject, mem, mem)

static int GetAssociatedMemObject(lua_State *L, cl_mem obj, cl_mem_info name)
    {
    void *value;
    cl_int ec = cl.GetMemObjectInfo(obj, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    if(value == 0) return 0;
    pushxxx(L, value);
    return 1;
    }

static int GetHostPtr(lua_State *L, cl_mem obj, cl_mem_info name)
    {
    void *value;
    cl_int ec = cl.GetMemObjectInfo(obj, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    if(value == 0) return 0;
    lua_pushlightuserdata(L, value);
    return 1;
    }



static int GetMemObjectInfo(lua_State *L)
    {
    cl_mem mem = checkmemobject(L, 1, NULL);
    cl_mem_info name = checkmeminfo(L, 2);
    switch(name)
        {
        case CL_MEM_TYPE: return GetMemType(L, mem, name);
        case CL_MEM_FLAGS: return GetFlags(L, mem, name);
        case CL_MEM_SIZE: return GetSize(L, mem, name);
        case CL_MEM_HOST_PTR: return GetHostPtr(L, mem, name);
        case CL_MEM_MAP_COUNT: return GetUint(L, mem, name);
        case CL_MEM_REFERENCE_COUNT: return GetUint(L, mem, name);
        case CL_MEM_CONTEXT: return GetContext(L, mem, name);
        case CL_MEM_ASSOCIATED_MEMOBJECT: return GetAssociatedMemObject(L, mem, name);
        case CL_MEM_OFFSET: return GetSize(L, mem, name);
        case CL_MEM_USES_SVM_POINTER: return GetBoolean(L, mem, name);
        default:
            return unexpected(L);
        }
    return 0;
    }


static const struct luaL_Reg Functions[] = 
    {
        { "get_mem_object_info", GetMemObjectInfo },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_mem(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }

