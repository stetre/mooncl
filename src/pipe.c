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

static int freepipe(lua_State *L, ud_t *ud)
    {
    cl_pipe pipe = (cl_pipe)ud->handle;
    if(!freeuserdata(L, ud, "pipe")) return 0;
    ReleaseAll(MemObject, MEM, pipe);
    return 0;
    }

static void DestructorCallback(cl_pipe pipe, void *luastate)
    {
#define L (lua_State*)luastate
    ud_t *ud = UD(pipe);
    if(!ud) return; /* already deleted */
    ud->destructor(L, ud);  
#undef L
    }


static int newpipe(lua_State *L, cl_context context, cl_pipe pipe)
    {
    ud_t *ud;
    ud = newuserdata(L, pipe, PIPE_MT, "pipe");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freepipe;  
    cl.SetMemObjectDestructorCallback(pipe, DestructorCallback, L);
    return 1;
    }

static int CreatePipe(lua_State *L)
    {
    cl_int ec;
    int err;
    cl_pipe pipe;
    const cl_pipe_properties *properties;
    cl_context context = checkcontext(L, 1, NULL);
    cl_mem_flags flags = checkflags(L, 2);
    cl_uint pipe_packet_size = luaL_checkinteger(L, 3);
    cl_uint pipe_max_packets = luaL_checkinteger(L, 4);

    CheckPfn_2_0(L, CreatePipe);
    properties = echeckpipeproperties(L, 5, &err);
    if(err < 0)
        return luaL_argerror(L, 5, errstring(err));
    if(err==ERR_NOTPRESENT) lua_pop(L, 1);
    pipe = cl.CreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, &ec);
    Free(L, (void*)properties);
    CheckError(L, ec);
    newpipe(L, context, pipe);
    return 1;
    }

GET_INFO_UINT(GetUint, Pipe, pipe, pipe)

static int GetPipeInfo(lua_State *L)
    {
    cl_pipe pipe = checkpipe(L, 1, NULL);
    cl_pipe_info name = checkpipeinfo(L, 2);

    CheckPfn_2_0(L, GetPipeInfo);
    switch(name)
        {
        case CL_PIPE_PACKET_SIZE: return GetUint(L, pipe, name);
        case CL_PIPE_MAX_PACKETS: return GetUint(L, pipe, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

RAW_FUNC(pipe)
TYPE_FUNC(pipe)
CONTEXT_FUNC(pipe)
PARENT_FUNC(pipe)
DELETE_FUNC(pipe)
RETAIN_FUNC(pipe, MemObject)
RELEASE_FUNC(pipe, MemObject, MEM)

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
        { "create_pipe", CreatePipe },
        { "get_pipe_info", GetPipeInfo },
        { "retain_pipe", Retain },
        { "release_pipe", Release },
        { NULL, NULL } /* sentinel */
    };

void mooncl_open_pipe(lua_State *L)
    {
    udata_define(L, PIPE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

