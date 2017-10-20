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
    lua_State *luastate;
    int called_submitted;
    int called_running;
    int called_complete;
    cl_int status_submitted;
    cl_int status_running;
    cl_int status_complete;
} udinfo_t;

static int freeevent(lua_State *L, ud_t *ud)
    {
    cl_event event = (cl_event)ud->handle;
    if(!freeuserdata(L, ud, "event")) return 0;
    ReleaseAll(Event, EVENT, event);
    return 0;
    }

int newevent(lua_State *L, cl_context context, cl_event event)
    {
    ud_t *ud;
    ud = newuserdata(L, event, EVENT_MT, "event");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freeevent;
    return 1;
    }

static int CreateUserEvent(lua_State *L)
    {
    cl_int ec;
    cl_context context = checkcontext(L, 1, NULL);
    cl_event event = cl.CreateUserEvent(context, &ec);
    CheckError(L, ec);
    newevent(L, context, event);
    return 1;
    }

static int WaitForEvents(lua_State *L)
    {
    cl_int ec;
    int err;
    cl_uint count;
    cl_event *event_list = checkeventlist(L, 1, &count, &err);
    if(err)
        return luaL_argerror(L, 1, errstring(err));
    ec = cl.WaitForEvents(count, event_list);
    Free(L, event_list);
    CheckError(L, ec);
    return 0;
    }

GET_INFO_OBJECT(GetCommandQueue, Event, event, event, queue)
GET_INFO_OBJECT(GetContext, Event, event, event, context)
GET_INFO_ENUM(GetCommandType, Event, event, event, pushcommandtype)
GET_INFO_UINT(GetUint, Event, event, event)

static int GetExecutionStatus(lua_State *L, cl_event obj)
    {
    cl_int ec;
    cl_int value;

    ec = cl.GetEventInfo(obj, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(value), &value, NULL);
    CheckError(L, ec);
    if(value < 0)
        lua_pushinteger(L, value);
    else
        pushexecutionstatus(L, value);
    return 1;
    }


static int GetEventInfo(lua_State *L)
    {
    cl_event event = checkevent(L, 1, NULL);
    cl_event_info name = checkeventinfo(L, 2);
    switch(name)
        {
        case CL_EVENT_COMMAND_QUEUE: return GetCommandQueue(L, event, name);
        case CL_EVENT_CONTEXT: return GetContext(L, event, name);
        case CL_EVENT_COMMAND_TYPE: return GetCommandType(L, event, name);
        case CL_EVENT_REFERENCE_COUNT: return GetUint(L, event, name);
        case CL_EVENT_COMMAND_EXECUTION_STATUS: return GetExecutionStatus(L, event);
        default:
            return unexpected(L);
        }
    return 0;
    }


static int SetUserEventStatus(lua_State *L)
    {
    cl_int ec, execution_status;
    cl_event event = checkevent(L, 1, NULL);
    if(lua_isinteger(L, 2))
        execution_status = lua_tointeger(L, 2); /* status may be a negative error code */
    else
        execution_status = checkexecutionstatus(L, 2); 
    ec = cl.SetUserEventStatus(event, execution_status);
    CheckError(L, ec);
    return 0;
    }

/* ----------------------------------------------------------------------- */

/* NOTE: The event callback must return promptly and be thread-safe.
 * For this reason, registering a user defined Lua callback to be called during
 * the C callback is unfeasible.
 *
 * Instead, in the C callback we just keep track of it having being called and
 * of the status information it receives, and we provide the user with a function
 * to poll for it.
 */
#define CALLBACK_FUNC(what)                                                     \
static void Callback_##what(cl_event event, cl_int status, void *user_data)     \
    {                                                                           \
    ud_t *ud = (ud_t*)user_data;                                                \
    udinfo_t *udinfo = (udinfo_t*)ud->info;                                     \
    if((event != (cl_event)ud->handle) || (udinfo == NULL)) return;             \
    udinfo->status_##what = status;                                             \
    udinfo->called_##what = 1;                                                  \
    }
CALLBACK_FUNC(submitted)
CALLBACK_FUNC(running)
CALLBACK_FUNC(complete)
#undef CALLBACK_FUNC

static int SetEventCallback(lua_State *L)
    {
    cl_int ec;
    ud_t *ud;
    cl_event event = checkevent(L, 1, &ud);
    cl_int type = checkexecutionstatus(L, 2);
    udinfo_t *udinfo = (udinfo_t*)ud->info;
    
    if(!udinfo)
        {
        udinfo = (udinfo_t*)Malloc(L, sizeof(udinfo_t));
        ud->info = udinfo;
        }
    
    switch(type)
        {
#define SET(what) do {                                              \
    udinfo->called_##what = 0;                                      \
    ec = cl.SetEventCallback(event, type, Callback_##what, ud);      \
} while(0)
        case CL_SUBMITTED:  SET(submitted); break;
        case CL_COMPLETE:   SET(complete); break;
        case CL_RUNNING:    SET(running); break;
        default:
            return luaL_argerror(L, 2, errstring(ERR_VALUE));
#undef SET
        }

    CheckError(L, ec);
    return 0;
    }
 
static int CheckEventCallback(lua_State *L)
    {
    cl_int status, type;
    ud_t *ud;
    udinfo_t *udinfo;

    checkevent(L, 1, &ud);
    type = checkexecutionstatus(L, 2);
    udinfo = (udinfo_t*)ud->info;
    
    if(!udinfo)
        return 0;
    
    switch(type)
        {
#define CHECK(what) do {                                            \
    if(!udinfo->called_##what) return 0;                            \
    udinfo->called_##what = 0;                                      \
    status = udinfo->status_##what;                                 \
} while(0)
        case CL_SUBMITTED:  CHECK(submitted); break;
        case CL_COMPLETE:   CHECK(complete); break;
        case CL_RUNNING:    CHECK(running); break;
        default:
            return luaL_argerror(L, 2, errstring(ERR_VALUE));
#undef CHECK
        }

    if(status < 0)
        lua_pushinteger(L, status);
    else
        pushexecutionstatus(L, status);
    return 1;
    }
 


/* ----------------------------------------------------------------------- */
GET_INFO_ULONG(GetProfiling, EventProfiling, event, profiling)

static int GetEventProfilingInfo(lua_State *L)
    {
    cl_event event = checkevent(L, 1, NULL);
    cl_profiling_info name = checkprofilinginfo(L, 2);
    switch(name)
        {
        case CL_PROFILING_COMMAND_QUEUED: return GetProfiling(L, event, name);
        case CL_PROFILING_COMMAND_SUBMIT: return GetProfiling(L, event, name);
        case CL_PROFILING_COMMAND_START: return GetProfiling(L, event, name);
        case CL_PROFILING_COMMAND_END: return GetProfiling(L, event, name);
        case CL_PROFILING_COMMAND_COMPLETE: return GetProfiling(L, event, name);
        default:
            return unexpected(L);
        }
    return 0;
    }


RAW_FUNC(event)
TYPE_FUNC(event)
CONTEXT_FUNC(event)
PARENT_FUNC(event)
DELETE_FUNC(event)
RETAIN_FUNC(event, Event)
RELEASE_FUNC(event, Event, EVENT)

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
        { "wait_for_events", WaitForEvents },
        { "get_event_info", GetEventInfo },
        { "create_user_event", CreateUserEvent },
        { "retain_event", Retain },
        { "release_event", Release },
        { "set_user_event_status", SetUserEventStatus },
        { "set_event_callback", SetEventCallback },
        { "check_event_callback", CheckEventCallback },
        { "get_event_profiling_info", GetEventProfilingInfo },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_event(lua_State *L)
    {
    udata_define(L, EVENT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

