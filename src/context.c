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
    ud_t *ud;
} udinfo_t;

static int freecontext(lua_State *L, ud_t *ud)
    {
    cl_context context = (cl_context)ud->handle;
    udinfo_t *udinfo = (udinfo_t*)ud->info;
    ud->info = NULL; /* we'll Free() it here */
    freechildren(L, COMMAND_QUEUE_MT, ud);
    freechildren(L, PROGRAM_MT, ud);
    freechildren(L, EVENT_MT, ud);
    freechildren(L, PIPE_MT, ud);
    freechildren(L, IMAGE_MT, ud);
    freechildren(L, BUFFER_MT, ud);
    freechildren(L, SAMPLER_MT, ud);
    freechildren(L, SVM_MT, ud);
    if(!freeuserdata(L, ud, "context")) return 0;
    ReleaseAll(Context, CONTEXT, context);
    Free(L, udinfo);
    return 0;
    }

static ud_t *newcontext(lua_State *L, cl_platform platform, cl_context context)
    {
    ud_t *ud;
    ud = newuserdata(L, context, CONTEXT_MT, "context");
    ud->platform = platform;
    ud->parent_ud = userdata(platform);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freecontext;
    return ud;
    }

typedef void (*NOTIFYp)(const char *errinfo, const void *private_info, size_t cb, void *userdata);
#define NO_CALLBACK (NOTIFYp)NULL

#if 1
#define Callback NO_CALLBACK
#else
static void Callback(const char *errinfo, const void *private_info, size_t cb, void *userdata) //@@
/* error notification (NB: must be thread-safe) */
    { 
#define udinfo ((udinfo_t*)userdata)
    ud_t *ud = udinfo->ud;
    cl_context context = (cl_context)ud->handle;

    // TODO store errinfo and private_info (whose length is cb) somewhere in udinfo
    //   (need a queue, maybe...)
    
    (void)context;
    (void)errinfo;
    (void)private_info;
    (void)cb;
#undef udinfo
    }

static int CheckContextCallback(lua_State *L)
/* errinfo, private_info = cl.check_context_callback(context) */
    {
    (void)L;
    TODO
    return 0;
    }
#endif

static int CreateContext(lua_State *L)
    {
    ud_t * ud;
    int err, register_callback;
    cl_int ec;
    cl_context context;
    cl_uint num_devices = 0;
    cl_device_type device_type;
    int from_type = 0;
    cl_device *devices = NULL;
    udinfo_t *udinfo = NULL;
    cl_context_properties *properties = NULL;
    cl_context_properties props[3];

    cl_platform platform = checkplatform(L, 1, NULL);
    props[0] = CL_CONTEXT_PLATFORM;
    props[1] = (cl_context_properties)platform;
    props[2] = 0;

#define CLEANUP() do {                       \
        if(properties) Free(L, properties);  \
        if(devices) Free(L, devices);        \
        if(udinfo) Free(L, udinfo);          \
} while(0)

    /* arg 2 may be either a cl_device_type (flags) or a list of devices (table) */
    if(lua_isinteger(L, 2))
        {
        from_type = 1;
        device_type = checkflags(L, 2);
        }
    else
        {
        devices = checkdevicelist(L, 2, &num_devices, &err);
        if(err < 0)
            return luaL_argerror(L, 2, errstring(err));
        }

    properties = echeckcontextproperties(L, 3, &err, platform);
    if(err < 0)
        { 
        CLEANUP(); 
        return luaL_argerror(L, 3, lua_tostring(L, -1));
        }
    if(err == ERR_NOTPRESENT) 
        lua_pop(L, 3); /* pop error string */

    register_callback = 0; //@@ optboolean(L, 4, 0);

    udinfo = (udinfo_t*)MallocNoErr(L, sizeof(udinfo_t));
    if(!udinfo)
        {
        CLEANUP();
        return luaL_error(L, errstring(ERR_MEMORY));
        }

    if(from_type)
        context = cl.CreateContextFromType(properties ? properties : props, device_type, 
                register_callback ? Callback : NO_CALLBACK, register_callback ? udinfo : NULL, &ec);
    else
        context = cl.CreateContext(properties ? properties : props, num_devices, devices, 
                register_callback ? Callback : NO_CALLBACK, register_callback ? udinfo : NULL, &ec);
    if(ec)
        {
        CLEANUP();
        CheckError(L, ec);
        return 0;
        }

    if(from_type)
        createcontextdevices(L, platform, context); //@@ this may fail, causing memory loss

    ud = newcontext(L, platform, context);
    if(udinfo)
        {
        udinfo->ud = ud;
        ud->info = udinfo;
        }
#undef CLEANUP
    return 1;
    }

GET_INFO_UINT(GetUint, Context, context, context)
GET_INFO_DEVICES(GetDevices, Context, context, context)

static int GetProperties(lua_State *L, cl_context obj)
    {
    cl_int ec;
    cl_context_properties *value;
    size_t size, n;

    ec = cl.GetContextInfo(obj, CL_CONTEXT_PROPERTIES, 0, NULL, &size);
    CheckError(L, ec);

    if(size == 0) 
        { lua_newtable(L); return 1; }

    n = size / sizeof(cl_context_properties);
    value = (cl_context_properties*)Malloc(L, n*sizeof(cl_context_properties));
    ec = cl.GetContextInfo(obj, CL_CONTEXT_PROPERTIES, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }
    
    pushcontextproperties(L, value);
    Free(L, value);
    return 1;
    }

static int GetContextInfo(lua_State *L)
    {
    cl_context context = checkcontext(L, 1, NULL);
    cl_context_info name = checkcontextinfo(L, 2);
    switch(name)
        {
        case CL_CONTEXT_REFERENCE_COUNT:    return GetUint(L, context, name);
        case CL_CONTEXT_NUM_DEVICES:    return GetUint(L, context, name);
        case CL_CONTEXT_DEVICES:    return GetDevices(L, context, name);
        case CL_CONTEXT_PROPERTIES: return GetProperties(L, context);
        default:
            return unexpected(L);
        }
    return 0;
    }

static int GetSupportedImageFormats(lua_State *L)
    {
    cl_int ec;
    cl_uint count, i;
    cl_image_format *image_formats;
    cl_context context = checkcontext(L, 1, NULL);
    cl_mem_flags flags = checkflags(L, 2);
    cl_mem_object_type image_type = checkmemobjecttype(L, 3);

    ec = cl.GetSupportedImageFormats(context, flags, image_type, 0, NULL, &count);
    CheckError(L, ec);
    
    lua_newtable(L);
    if(count == 0) return 1;

    image_formats = (cl_image_format*)Malloc(L, count * sizeof(cl_image_format));
    
    ec = cl.GetSupportedImageFormats(context, flags, image_type, count, image_formats, NULL);
    if(ec)
        {
        Free(L, image_formats);
        CheckError(L, ec);
        return 0;
        }
    
    for(i=0; i<count; i++)
        {
        pushimageformat(L, &image_formats[i]);
        lua_rawseti(L, -2, i+1);
        }
    
    Free(L, image_formats);
    return 1;
    }

static int TerminateContext(lua_State *L)
    {
    cl_int ec;
    ud_t *ud;
    cl_context context = checkcontext(L, 1, &ud);
    CheckExtPfn(L, ud, TerminateContextKHR);
    ec = ud->clext->TerminateContextKHR(context);
    CheckError(L, ec);
    return 0;
    }

RAW_FUNC(context)
TYPE_FUNC(context)
PLATFORM_FUNC(context)
PARENT_FUNC(context)
DELETE_FUNC(context)
RETAIN_FUNC(context, Context)
RELEASE_FUNC(context, Context, CONTEXT)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "platform", Platform },
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
        { "create_context", CreateContext },
//      { "check_context_callback", CheckContextCallback },
        { "retain_context", Retain },
        { "release_context", Release },
        { "get_context_info", GetContextInfo },
        { "get_supported_image_formats", GetSupportedImageFormats },
        { "terminate_context", TerminateContext },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_context(lua_State *L)
    {
    udata_define(L, CONTEXT_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

