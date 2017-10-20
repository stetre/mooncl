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

static int freequeue(lua_State *L, ud_t *ud)
    {
    cl_queue queue = (cl_queue)ud->handle;
    if(!freeuserdata(L, ud, "queue")) return 0;
    cl.Finish(queue);
    ReleaseAll(CommandQueue, QUEUE, queue);
    return 0;
    }

static int newqueue(lua_State *L, cl_queue queue, cl_context context, cl_device device)
    {
    ud_t *ud;
    ud = newuserdata(L, queue, COMMAND_QUEUE_MT, "queue");
    ud->platform = UD(context)->platform;
    ud->context = context;
    ud->device = device;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freequeue;  
    return 1;
    }


static int CreateCommandQueue(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_command_queue_properties propflags;
    cl_queue_properties *properties = NULL;
    cl_queue queue; 

    cl_context context = checkcontext(L, 1, NULL);
    cl_device device = checkdevice(L, 2, NULL);

    properties = echeckqueueproperties(L, 3, &propflags, &err);
    if(err < 0)
        return luaL_argerror(L, 3, lua_tostring(L, -1));
    if(err == ERR_NOTPRESENT) 
        lua_pop(L, 1); /* pop error string */

    if(cl.CreateCommandQueueWithProperties) /* >= VERSION_2_0 */
        queue = cl.CreateCommandQueueWithProperties(context, device, properties, &ec); 
    else /* revert to VERSION_1_2 */
        queue = cl.CreateCommandQueue(context, device, propflags, &ec);
    
    if(ec)
        {
        Free(L, properties);
        CheckError(L, ec);
        return 0;
        }

    newqueue(L, queue, context, device);
    return 1;
    }

GET_INFO_OBJECT(GetContext, CommandQueue, queue, queue, context)
GET_INFO_OBJECT(GetDevice, CommandQueue, queue, queue, device)
GET_INFO_UINT(GetUint, CommandQueue, queue, queue)
GET_INFO_FLAGS(GetFlags, CommandQueue, queue, queue)
GET_INFO_OBJECT(GetCommandQueue, CommandQueue, queue, queue, queue)

static int GetCommandQueueInfo(lua_State *L)
    {
    cl_queue queue = checkqueue(L, 1, NULL);
    cl_command_queue_info name = checkcommandqueueinfo(L, 2);
    switch(name)
        {
        case CL_QUEUE_CONTEXT: return GetContext(L, queue, name);
        case CL_QUEUE_DEVICE: return GetDevice(L, queue, name);
        case CL_QUEUE_REFERENCE_COUNT: return GetUint(L, queue, name);
        case CL_QUEUE_PROPERTIES: return GetFlags(L, queue, name);
        case CL_QUEUE_SIZE: return GetUint(L, queue, name);
        case CL_QUEUE_DEVICE_DEFAULT: return GetCommandQueue(L, queue, name);//CL_VERSION_2_1
        default:
            return unexpected(L);
        }
    return 0;
    }

RAW_FUNC(queue)
TYPE_FUNC(queue)
PLATFORM_FUNC(queue)
DEVICE_FUNC(queue)
CONTEXT_FUNC(queue)
PARENT_FUNC(queue)
DELETE_FUNC(queue)
RETAIN_FUNC(queue, CommandQueue)
RELEASE_FUNC(queue, CommandQueue, QUEUE)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "platform", Platform },
        { "device", Device },
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
        { "create_command_queue", CreateCommandQueue },
        { "retain_command_queue", Retain },
        { "release_command_queue", Release },
        { "get_command_queue_info", GetCommandQueueInfo },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_queue(lua_State *L)
    {
    udata_define(L, COMMAND_QUEUE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

