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

typedef struct {
    size_t size;
    size_t origin;
    cl_mem_flags flags;
} udinfo_t;

int testbufferboundaries(lua_State *L, cl_buffer buffer, size_t offset, size_t size)
    {
    udinfo_t *udinfo;
    ud_t *ud = UD(buffer);
    if(!ud) return unexpected(L);
    udinfo = (udinfo_t*)ud->info;
    if((offset >= udinfo->size) || (size > (udinfo->size - offset)))
        return 0;
    return 1;
    }

int checkbufferboundaries(lua_State *L, cl_buffer buffer, size_t offset, size_t size)
    {
    if(!testbufferboundaries(L, buffer, offset, size))
        return luaL_error(L, errstring(ERR_BOUNDARIES));
    return 1;
    }

static int freebuffer(lua_State *L, ud_t *ud)
    {
    cl_buffer buffer = (cl_buffer)ud->handle;
    int sub_buffer = IsSubBuffer(ud);
    freechildren(L, BUFFER_MT, ud); /* sub buffers */
    if(!freeuserdata(L, ud, sub_buffer ? "sub buffer" : "buffer")) return 0;
    ReleaseAll(MemObject, MEM, buffer);
    return 0;
    }

static void DestructorCallback(cl_buffer buffer, void *luastate)
    {
#define L (lua_State*)luastate
    ud_t *ud = UD(buffer);
    if(!ud) return; /* already deleted */
    ud->destructor(L, ud);  
#undef L
    }

static ud_t *newbuffer(lua_State *L, cl_context context, cl_buffer buffer, void *udinfo)
    {
    ud_t *ud;
    ud = newuserdata(L, buffer, BUFFER_MT, "buffer");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freebuffer;  
    ud->info = udinfo;
    cl.SetMemObjectDestructorCallback(buffer, DestructorCallback, L);
    return ud;
    }

static ud_t *newsubbuffer(lua_State *L, cl_buffer parent, cl_buffer buffer, void *udinfo)
    {
    ud_t *ud;
    ud = newuserdata(L, buffer, BUFFER_MT, "sub buffer");
    ud->context = UD(parent)->context;
    ud->parent_ud = UD(parent);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freebuffer;  
    ud->info = udinfo;
    MarkSubBuffer(ud);
    cl.SetMemObjectDestructorCallback(buffer, DestructorCallback, L);
    return ud;
    }

static int CreateBuffer(lua_State *L)
    {
    cl_int ec;
    udinfo_t *udinfo;
    cl_buffer buffer;
    size_t size;
    void *host_ptr;

    cl_context context = checkcontext(L, 1, NULL);
    cl_mem_flags flags = checkflags(L, 2);

//  if(lua_type(L, 3) == LUA_TSTRING)
//      {
//      if(flags & CL_MEM_USE_HOST_PTR)
//          return luaL_argerror(L, 3, "the 'use host ptr' flag requires a pointer to valid memory");
//      host_ptr = (void*)luaL_checklstring(L, 3, &size);
//      if(size == 0)
//          return luaL_argerror(L, 3, "zero length initializing data");
//      }
//  else
//      {
        size = luaL_checkinteger(L, 3);
        host_ptr = optlightuserdata(L, 4);
//      }

    udinfo = (udinfo_t*)MallocNoErr(L, sizeof(udinfo_t));
    if(!udinfo)
        return luaL_error(L, errstring(ERR_MEMORY));
    udinfo->flags = flags;
    udinfo->origin = 0;
    udinfo->size = size;

//  DBG("host_ptr = %p\n",host_ptr);
    buffer = cl.CreateBuffer(context, flags, size, host_ptr, &ec);
    if(ec)
        {
        Free(L, udinfo);
        CheckError(L, ec);
        return 0;
        }
    
    newbuffer(L, context, buffer, udinfo);
    return 1;
    }

static int CreateBufferRegion(lua_State *L) /* CreateSubBuffer */
    {
    cl_int ec;
    ud_t *ud, *parent_ud;
    udinfo_t *udinfo, *parent_udinfo;
    cl_buffer buffer;
    cl_buffer_region region;
    cl_buffer parent_buffer = checkbuffer(L, 1, &parent_ud);
    cl_mem_flags flags = checkflags(L, 2);
    region.origin = luaL_checkinteger(L, 3);
    region.size = luaL_checkinteger(L, 4);

    if(IsSubBuffer(parent_ud))
        return luaL_argerror(L, 1, "cannot create a sub buffer fro another sub buffer");

    parent_udinfo = (udinfo_t*)parent_ud->info;
    if(region.origin >= parent_udinfo->size)
        return luaL_argerror(L, 3, "out of range");
    if(region.size > (parent_udinfo->size - region.origin))
        return luaL_argerror(L, 4, "out of range");

    udinfo = (udinfo_t*)Malloc(L, sizeof(udinfo_t));
    udinfo->origin = region.origin;
    udinfo->size = region.size;
    udinfo->flags = flags;

    buffer = cl.CreateSubBuffer(parent_buffer, flags, CL_BUFFER_CREATE_TYPE_REGION, &region, &ec);
    if(ec)
        {
        Free(L, udinfo);
        CheckError(L, ec);
        return 0;
        }

    ud = newsubbuffer(L, parent_buffer, buffer, udinfo);
    MarkBufferRegion(ud);
    return 1;
    }

static int CreateFromGLBuffer(lua_State *L)
    {
    ud_t *ud, *context_ud;
    cl_int ec;
    udinfo_t *udinfo;
    cl_buffer buffer;
    size_t size;

    cl_context context = checkcontext(L, 1, &context_ud);
    cl_mem_flags flags = checkflags(L, 2);
    cl_GLuint bufobj = luaL_checkinteger(L, 3);

    CheckExtPfn(L, context_ud, CreateFromGLBuffer);

    udinfo = (udinfo_t*)MallocNoErr(L, sizeof(udinfo_t));
    if(!udinfo)
        return luaL_error(L, errstring(ERR_MEMORY));
    udinfo->flags = flags;
    udinfo->origin = 0;
    //udinfo->size = size;

    buffer = context_ud->clext->CreateFromGLBuffer(context, flags, bufobj, &ec);
    if(ec)
        {
        Free(L, udinfo);
        CheckError(L, ec);
        return 0;
        }

    /* Retrieve size for boundary checks */
    ec = cl.GetMemObjectInfo(buffer, CL_MEM_SIZE, sizeof(size), &size, NULL);
    if(ec)
        {
        Free(L, udinfo);
        ReleaseAll(MemObject, MEM, buffer);
        CheckError(L, ec);
        return 0;
        }
    udinfo->size = size;
    
    ud = newbuffer(L, context, buffer, udinfo);
    MarkGLBuffer(ud);
    return 1;
    }


RAW_FUNC(buffer)
TYPE_FUNC(buffer)
CONTEXT_FUNC(buffer)
PARENT_FUNC(buffer)
DELETE_FUNC(buffer)
RETAIN_FUNC(buffer, MemObject)
RELEASE_FUNC(buffer, MemObject, MEM)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "context", Context },
        { "parent", Parent },
        { "delete", Delete },
        { NULL, NULL } /* sentinel */
    };


static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };


static const struct luaL_Reg Functions[] = 
    {
        { "create_buffer", CreateBuffer },
        { "create_buffer_region", CreateBufferRegion },
        { "create_from_gl_buffer", CreateFromGLBuffer },
        { "retain_buffer", Retain },
        { "release_buffer", Release },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_buffer(lua_State *L)
    {
    udata_define(L, BUFFER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

