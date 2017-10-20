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

#define ECHECK_PREAMBLE                                                         \
    memset(p, 0, sizeof(*p));                                                   \
    if(lua_isnoneornil(L, arg))                                                 \
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); return ERR_NOTPRESENT; }\
    if(lua_type(L, arg) != LUA_TTABLE)                                          \
        { lua_pushstring(L, errstring(ERR_TABLE)); return ERR_TABLE; }
        
#define POPERROR()  lua_pop(L, 1)

#define PUSHFIELD(sname)    \
    do { lua_pushstring(L, sname); lua_rawget(L, arg); arg1 = lua_gettop(L); } while(0)

#define POPFIELD()          \
    do { lua_remove(L, arg1); } while(0)

#define GetIntegerOpt(name, sname, defval) do {     \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isinteger(L, -1))                        \
        p->name = lua_tointeger(L, -1);             \
    else if(lua_isnoneornil(L, -1))                 \
        p->name = defval;                           \
    else                                            \
        err = ERR_TYPE;                             \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetInteger(name, sname) GetIntegerOpt(name, sname, 0)

#define GetEnum(name, sname, testfunc) do {         \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    p->name = testfunc(L, -1, &err);                \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)

#define GetObjectOpt(name, sname, ttt) do {         \
/* eg: cl_buffer -> ttt = buffer */                 \
    lua_pushstring(L, sname);                       \
    lua_rawget(L, arg);                             \
    err = 0;                                        \
    if(lua_isnoneornil(L, -1))                      \
        p->name = 0;                                \
    else                                            \
        {                                           \
        p->name = test##ttt(L, -1, NULL);           \
        if(!p->name) err = ERR_TYPE;                \
        }                                           \
    lua_pop(L, 1);                                  \
    if(err)                                         \
        return fielderror(L, sname, err);           \
} while(0)


static int fielderror(lua_State *L, const char *fieldname, int errcode)
    { 
    lua_pushfstring(L, "%s: %s", fieldname, errstring(errcode)); 
    return ERR_GENERIC; 
    }

#define SetInteger(name, sname) do { lua_pushinteger(L, p->name); lua_setfield(L, -2, sname); } while(0)
#define SetEnum(name, sname, pushfunc) do { pushfunc(L, p->name); lua_setfield(L, -2, sname); } while(0)

/*-----------------------------------------------------------------------------*/

cl_device_partition_property *echeckdevicepartitionproperty(lua_State *L, int arg, int *err)
    {
    int arg1;
    cl_uint n, i;
    cl_device_partition_property *p;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); *err=ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); *err = ERR_TABLE; return NULL; }

#define F "equally"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        if(!lua_isinteger(L, arg1))
            { lua_pushfstring(L, "%s: not an integer", F); *err = ERR_VALUE; return NULL; }
        p = (cl_device_partition_property*)Malloc(L, 3*sizeof(cl_device_partition_property));
        p[0] = CL_DEVICE_PARTITION_EQUALLY;
        p[1] = lua_tointeger(L, arg1);
        p[2] = 0;
        POPFIELD();
        return p;
        }
    POPFIELD(); 
#undef F
    
#define F "by_affinity_domain"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        if(!lua_isinteger(L, arg1))
            { lua_pushfstring(L, "%s: not an integer", F); *err = ERR_VALUE; return NULL; }
        p = (cl_device_partition_property*)Malloc(L, 3*sizeof(cl_device_partition_property));
        p[0] = CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN;
        p[1] = lua_tointeger(L, arg1);
        p[2] = 0;
        POPFIELD();
        return p;
        }
    POPFIELD(); 
#undef F
        
#define F "by_counts"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
#define ERR() do {  \
    lua_pushfstring(L, "%s: not a table of integers", F); *err = ERR_VALUE; return NULL; \
} while(0);
        if(lua_type(L, arg1) != LUA_TTABLE) ERR();
        n = luaL_len(L, arg1);
        if(n == 0) ERR();
        p = (cl_device_partition_property*)Malloc(L, (n+3)*sizeof(cl_device_partition_property));
        p[0] = CL_DEVICE_PARTITION_BY_COUNTS;
        for(i=1; i <= n; i++)
            {
            lua_rawgeti(L, arg1, i);
            if(!lua_isinteger(L, -1))
                {
                Free(L, p);
                lua_pop(L, 1);
                POPFIELD();
                ERR();
                }
            p[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
            }
        p[n+1] = CL_DEVICE_PARTITION_BY_COUNTS_LIST_END;
        p[n+2] = 0;
        POPFIELD();
        return p;
#undef ERR
        }
    POPFIELD(); 
#undef F
    lua_pushstring(L, errstring(ERR_EMPTY)); 
    *err=ERR_EMPTY;
    return NULL;
    }
    

int pushdevicepartitionproperty(lua_State *L, cl_device_partition_property *p)
    {
    int i;
    if(!p || (p[0] == 0))
        {
        lua_pushnil(L);
        return 1;
        }
    lua_newtable(L);
    switch(p[0])
        {
        case CL_DEVICE_PARTITION_EQUALLY:
                    lua_pushinteger(L, p[1]); lua_setfield(L, -2, "equally");
                    return 1;
        case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                    lua_pushinteger(L, p[1]); lua_setfield(L, -2, "by_affinity_domain");
                    return 1;
        case CL_DEVICE_PARTITION_BY_COUNTS:
                    lua_newtable(L);
                    i = 1;
                    while(p[i]!=0)
                        {
                        lua_pushinteger(L, p[i]);
                        lua_rawseti(L, -2, i);
                        i++;
                        }
                    lua_setfield(L, -2, "by_counts");
                    return 1;
        default:
            return luaL_error(L, "unknown cl_device_partition_property 0x%.8lx\n", p[0]);
        }
    return 0;
    }

/*-----------------------------------------------------------------------------*/

cl_context_properties *echeckcontextproperties(lua_State *L, int arg, int *err, cl_platform use_platform)
    {
    int arg1;
    cl_uint n, i;
    cl_context_properties *p;
    cl_platform_id platform = 0;
    intptr_t interop_user_sync = -1;
    intptr_t gl_context = -1;
    intptr_t cgl_sharegroup = -1;
    intptr_t glx_display = -1;
    intptr_t egl_display = -1;
    intptr_t wgl_hdc = -1;
    intptr_t context_terminate = -1;

    *err = 0;
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); *err=ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); *err = ERR_TABLE; return NULL; }

    n = 0;

    if(use_platform)
        {   
        platform = use_platform;
        n += 2;
        }
    else
#define F "platform"
        {
        PUSHFIELD(F);
        if(!lua_isnoneornil(L, arg1))
            {
            platform = testplatform(L, arg1, NULL);
            if(!platform)
                {
                POPFIELD(); 
                lua_pushfstring(L, "%s: not a platform", F); 
                *err = ERR_VALUE; 
                return NULL;
                }
            n += 2;
            }
        POPFIELD();
        }
#undef F
#define F "interop_user_sync"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { interop_user_sync = lua_toboolean(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "gl_context"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { gl_context = (intptr_t)checklightuserdataorzero(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "cgl_sharegroup"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { cgl_sharegroup = (intptr_t)checklightuserdataorzero(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "glx_display"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { glx_display = (intptr_t)checklightuserdataorzero(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "egl_display"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { egl_display = (intptr_t)checklightuserdataorzero(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "wgl_hdc"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { wgl_hdc = (intptr_t)checklightuserdataorzero(L, arg1); n += 2; }
    POPFIELD();
#undef F
#define F "context_terminate"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { context_terminate = checkboolean(L, arg1); n += 2; }
    POPFIELD();
#undef F
#if 0
CL_CONTEXT_MEMORY_INITIALIZE_KHR @@
CL_CONTEXT_D3D10_DEVICE_KHR
CL_CONTEXT_D3D11_DEVICE_KHR
CL_CONTEXT_ADAPTER_D3D9_KHR
CL_CONTEXT_ADAPTER_D3D9EX_KHR
CL_CONTEXT_ADAPTER_DXVA_KHR
#endif

   if(n == 0)
        {
        lua_pushstring(L, errstring(ERR_NOTPRESENT)); 
        *err=ERR_NOTPRESENT;
        return NULL;
        }

    p = (cl_context_properties*)Malloc(L, (n+1)*sizeof(cl_context_properties));
    i = 0;
    if(platform != 0)
        {
        p[i++] = CL_CONTEXT_PLATFORM;
        p[i++] = (cl_context_properties)platform;
        }
    if(interop_user_sync != -1)
        {
        p[i++] = CL_CONTEXT_INTEROP_USER_SYNC;
        p[i++] = interop_user_sync;
        }
    if(gl_context != -1)
        {
        p[i++] = CL_GL_CONTEXT_KHR;
        p[i++] = gl_context;
        }
    if(cgl_sharegroup != -1)
        {
        p[i++] = CL_CGL_SHAREGROUP_KHR;
        p[i++] = cgl_sharegroup;
        }
    if(glx_display != -1)
        {
        p[i++] = CL_GLX_DISPLAY_KHR;
        p[i++] = glx_display;
        }
    if(egl_display != -1)
        {
        p[i++] = CL_EGL_DISPLAY_KHR;
        p[i++] = egl_display;
        }
    if(wgl_hdc != -1)
        {
        p[i++] = CL_WGL_HDC_KHR;
        p[i++] = wgl_hdc ;
        }
    if(context_terminate != -1)
        {
        p[i++] = CL_CONTEXT_TERMINATE_KHR;
        p[i++] = context_terminate;
        }

    p[i] = 0;
    return p;
    }


int pushcontextproperties(lua_State *L, cl_context_properties *p)
    {
    int i;
    if(!p || p[0] == 0)
        {
        lua_pushnil(L);
        return 1;
        }
    lua_newtable(L);
    i = 0;
    while(p[i] != 0)
        {
        switch(p[i])
            {
            case CL_CONTEXT_PLATFORM:   
                        pushplatform(L, (cl_platform)(p[++i]));
                        lua_setfield(L, -2, "platform");
                        break;
            case CL_CONTEXT_INTEROP_USER_SYNC:
                        lua_pushboolean(L, p[++i]);
                        lua_setfield(L, -2, "interop_user_sync");
                        break;
            case CL_GL_CONTEXT_KHR:
                        if(p[++i]==0) lua_pushinteger(L, 0); 
                        else lua_pushlightuserdata(L, (void*)(p[i]));
                        lua_setfield(L, -2, "gl_context");
                        break;
            case CL_CGL_SHAREGROUP_KHR:
                        if(p[++i]==0) lua_pushinteger(L, 0); 
                        else lua_pushlightuserdata(L, (void*)(p[i]));
                        lua_setfield(L, -2, "cgl_sharegroup");
                        break;
            case CL_GLX_DISPLAY_KHR:
                        if(p[++i]==0) lua_pushinteger(L, 0); 
                        else lua_pushlightuserdata(L, (void*)(p[i]));
                        lua_setfield(L, -2, "glx_display");
                        break;
            case CL_EGL_DISPLAY_KHR:
                        if(p[++i]==0) lua_pushinteger(L, 0); 
                        else lua_pushlightuserdata(L, (void*)(p[i]));
                        lua_setfield(L, -2, "egl_display");
                        break;
            case CL_WGL_HDC_KHR:
                        if(p[++i]==0) lua_pushinteger(L, 0); 
                        else lua_pushlightuserdata(L, (void*)(p[i]));
                        lua_setfield(L, -2, "wgl_hdc");
                        break;
            case CL_CONTEXT_TERMINATE_KHR:   
                        lua_pushboolean(L, (p[++i]));
                        lua_setfield(L, -2, "context_terminate");
                        break;
            default:
                return luaL_error(L, "unknown cl_context_properties 0x%.8lx\n", p[i]);
            }
        i++;
        }
    return 1;
    }


/*-----------------------------------------------------------------------------*/


cl_queue_properties *echeckqueueproperties(lua_State *L, int arg, cl_command_queue_properties *propflags, int *err)
    {
    int arg1;
    cl_uint n, i;
    cl_queue_properties *p;
    cl_command_queue_properties queue_props = 0;
    cl_uint queue_size;
    int queue_props_present = 0;
    int queue_size_present = 0;
    cl_queue_priority_khr queue_priority = -1;
    cl_queue_throttle_khr queue_throttle = -1;

    *err = 0;

    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); *err=ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); *err = ERR_TABLE; return NULL; }

    n = 0;
#define F "properties"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        if(!lua_isinteger(L, arg1))
            { lua_pushfstring(L, "%s: not an integer", F); *err = ERR_VALUE; return NULL; }
        queue_props = checkflags(L, arg1);
        queue_props_present = 1;
        n += 2;
        }
    POPFIELD(); 
#undef F
#define F "size"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        if(!lua_isinteger(L, arg1))
            { lua_pushfstring(L, "%s: not an integer", F); *err = ERR_VALUE; return NULL; }
        queue_size = lua_tointeger(L, arg1);
        queue_size_present = 1;
        n += 2;
        }
    POPFIELD(); 
#undef F
#define F "queue_priority"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { queue_priority = checkqueuepriority(L, arg1); n += 2; }
    POPFIELD(); 
#undef F
#define F "queue_throttle"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        { queue_throttle = checkqueuethrottle(L, arg1); n += 2; }
    POPFIELD(); 
#undef F

    if(n == 0)
        {
        lua_pushstring(L, errstring(ERR_NOTPRESENT)); 
        *err=ERR_NOTPRESENT;
        return NULL;
        }

    p = (cl_queue_properties*)Malloc(L, (n+1)*sizeof(cl_queue_properties));
    i = 0;
    if(queue_props_present)
        {
        p[i++] = CL_QUEUE_PROPERTIES;
        p[i++] = (cl_queue_properties)queue_props;
        }
    if(queue_size_present)
        {
        p[i++] = CL_QUEUE_SIZE;
        p[i++] = (cl_queue_properties)queue_size;
        }
    if(queue_priority != (cl_queue_priority_khr)-1)
        {
        p[i++] = CL_QUEUE_PRIORITY_KHR;
        p[i++] = (cl_queue_properties)queue_priority;
        }
    if(queue_throttle != (cl_queue_throttle_khr)-1)
        {
        p[i++] = CL_QUEUE_THROTTLE_KHR;
        p[i++] = (cl_queue_properties)queue_throttle;
        }
    p[i] = 0;
    
    if(propflags) 
        *propflags = queue_props; /* used by caller if version 2.0 is not supported */
    return p;
    }


int pushqueueproperties(lua_State *L, cl_queue_properties *p)
    {
    int i;
    if(!p || p[0] == 0)
        {
        lua_pushnil(L);
        return 1;
        }
    lua_newtable(L);
    i = 0;
    while(p[i] != 0)
        {
        switch(p[i])
            {
            case CL_QUEUE_PROPERTIES:
                        pushflags(L, (cl_bitfield)(p[++i]));
                        lua_setfield(L, -2, "properties");
                        break;
            case CL_QUEUE_SIZE:
                        lua_pushinteger(L, p[++i]);
                        lua_setfield(L, -2, "size");
                        break;
            case CL_QUEUE_PRIORITY_KHR:
                        pushqueuepriority(L, p[++i]);
                        lua_setfield(L, -2, "queue_priority");
                        break;
            case CL_QUEUE_THROTTLE_KHR :
                        pushqueuethrottle(L, p[++i]);
                        lua_setfield(L, -2, "queue_throttle");
                        break;
            default:
                return luaL_error(L, "unknown cl_queue_properties 0x%.8lx\n", p[i]);
            }
        i++;
        }
    return 1;
    }

/*-----------------------------------------------------------------------------*/

cl_sampler_properties *echecksamplerproperties(lua_State *L, int arg, 
    cl_bool *v1_2_normalized_coords, 
    cl_addressing_mode *v1_2_addressing_mode,
    cl_filter_mode *v1_2_filter_mode,
    int *err)
    {
    int arg1;
    cl_uint n, i;
    cl_sampler_properties *p;
    cl_bool normalized_coords = 0;
    cl_addressing_mode addressing_mode = 0;
    cl_filter_mode filter_mode = 0;
    int normalized_coords_present = 0;
    int addressing_mode_present = 0;
    int filter_mode_present = 0;
    
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); *err=ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); *err = ERR_TABLE; return NULL; }

    n = 0;
#define F "normalized_coords"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        normalized_coords = lua_toboolean(L, arg1);
        normalized_coords_present = 1;
        n += 2;
        }
    POPFIELD(); 
#undef F

#define F "addressing_mode"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        addressing_mode = checkaddressingmode(L, arg1);
        addressing_mode_present = 1;
        n += 2;
        }
    POPFIELD(); 
#undef F

#define F "filter_mode"
    PUSHFIELD(F);
    if(!lua_isnoneornil(L, arg1))
        {
        filter_mode = checkfiltermode(L, arg1);
        filter_mode_present = 1;
        n += 2;
        }
    POPFIELD(); 
#undef F

    if(n == 0)
        {
        lua_pushstring(L, errstring(ERR_NOTPRESENT)); 
        *err=ERR_NOTPRESENT;
        return NULL;
        }

    p = (cl_sampler_properties*)Malloc(L, (n+1)*sizeof(cl_sampler_properties));
    i = 0;
    if(normalized_coords_present)
        {
        p[i++] = CL_SAMPLER_NORMALIZED_COORDS;
        p[i++] = normalized_coords;
        }
    if(addressing_mode_present)
        {
        p[i++] = CL_SAMPLER_ADDRESSING_MODE;
        p[i++] = addressing_mode;
        }
    if(filter_mode_present)
        {
        p[i++] = CL_SAMPLER_FILTER_MODE;
        p[i++] = filter_mode;
        }
    p[i] = 0;
    
    if(v1_2_normalized_coords) *v1_2_normalized_coords = normalized_coords;
    if(v1_2_addressing_mode) *v1_2_addressing_mode = addressing_mode;
    if(v1_2_filter_mode) *v1_2_filter_mode = filter_mode;
    return p;
    }


int pushsamplerproperties(lua_State *L, cl_sampler_properties *p)
    {
    int i;
    if(!p || p[0] == 0)
        {
        lua_pushnil(L);
        return 1;
        }
    lua_newtable(L);
    i = 0;
    while(p[i] != 0)
        {
        switch(p[i])
            {
            case CL_SAMPLER_NORMALIZED_COORDS:  
                        lua_pushboolean(L, p[++i]);
                        lua_setfield(L, -2, "normalized_coords");
                        break;
            case CL_SAMPLER_ADDRESSING_MODE:
                        pushaddressingmode(L, p[++i]);
                        lua_setfield(L, -2, "addressing_mode");
                        break;
            case CL_SAMPLER_FILTER_MODE:
                        pushfiltermode(L, p[++i]);
                        lua_setfield(L, -2, "filter_mode");
                        break;
            default:
                return luaL_error(L, "unknown cl_sampler_properties 0x%.8lx\n", p[i]);
            }
        i++;
        }
    return 1;
    }

/*-----------------------------------------------------------------------------*/

cl_pipe_properties *echeckpipeproperties(lua_State *L, int arg, int *err)
    {
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { lua_pushstring(L, errstring(ERR_NOTPRESENT)); *err=ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { lua_pushstring(L, errstring(ERR_TABLE)); *err = ERR_TABLE; return NULL; }
    return NULL;
    }


int pushpipeproperties(lua_State *L, cl_pipe_properties *p)
    {
    if(!p || p[0] == 0)
        {
        lua_pushnil(L);
        return 1;
        }
    lua_newtable(L);
    return 1;
    }

/*-----------------------------------------------------------------------------*/

int echeckimageformat(lua_State *L, int arg, cl_image_format *p)
    {
    int err;
    ECHECK_PREAMBLE;
    GetEnum(image_channel_order, "channel_order", testchannelorder);
    GetEnum(image_channel_data_type, "channel_type", testchanneltype);
    return 0;
    }

int pushimageformat(lua_State *L, cl_image_format *p)
    {
    if(!p) 
        { lua_pushnil(L); return 1; }
    lua_newtable(L);
    SetEnum(image_channel_order, "channel_order", pushchannelorder);
    SetEnum(image_channel_data_type, "channel_type", pushchanneltype);
    return 1;
    }

/*-----------------------------------------------------------------------------*/

int echeckimagedesc(lua_State *L, int arg, cl_image_desc *p)
    {
    int err;
    ECHECK_PREAMBLE;
    GetEnum(image_type, "type", testmemobjecttype);
    GetIntegerOpt(image_width, "width", 1);
    GetIntegerOpt(image_height, "height", 1);
    GetIntegerOpt(image_depth, "depth", 1);
    GetIntegerOpt(image_array_size, "array_size", 1);
    GetIntegerOpt(image_row_pitch, "row_pitch", 0);
    GetIntegerOpt(image_slice_pitch, "slice_pitch", 0);
    GetIntegerOpt(num_mip_levels, "num_mip_levels", 0);
    GetIntegerOpt(num_samples, "num_samples", 0);
    GetObjectOpt(mem_object, "buffer", buffer);
    if(p->mem_object == NULL)
        GetObjectOpt(mem_object, "image", image);
    return 0;
    }

