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



static int GetGLObjectType(lua_State *L)
    {
    cl_int ec;
    ud_t *ud;
    cl_gl_object_type val;
    cl_mem gl_object = checkmemobject(L, 1, &ud);
    if(!IsGLObject(ud)) 
        return luaL_argerror(L, 1, "not a gl object");
    CheckExtPfn(L, ud, GetGLObjectInfo);
    ec = ud->clext->GetGLObjectInfo(gl_object, &val, NULL);
    CheckError(L, ec);
    pushglobjecttype(L, val);
    return 1;
    }

static int GetGLObjectName(lua_State *L)
    {
    cl_int ec;
    ud_t *ud;
    cl_uint val;
    cl_mem gl_object = checkmemobject(L, 1, &ud);
    if(!IsGLObject(ud)) 
        return luaL_argerror(L, 1, "not a gl object");
    CheckExtPfn(L, ud, GetGLObjectInfo);
    ec = ud->clext->GetGLObjectInfo(gl_object, NULL, &val);
    CheckError(L, ec);
    lua_pushinteger(L, val);
    return 1;
    }

/*---------------------------------------------------------------------------*/

static int GetTextureInfoInt(lua_State *L, cl_mem obj, ud_t *ud, cl_gl_texture_info name)
    {
    cl_int value;
    cl_int ec = ud->clext->GetGLTextureInfo(obj, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetTextureInfoTarget(lua_State *L, cl_mem obj, ud_t *ud, cl_gl_texture_info name)
    {
    cl_uint value;
    cl_int ec = ud->clext->GetGLTextureInfo(obj, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    pushgltexturetarget(L, value);
    return 1;
    }

static int GetTextureInfoSize(lua_State *L, cl_mem obj, ud_t *ud, cl_gl_texture_info name)
    {
    size_t value; //GLsizei 
    cl_int ec = ud->clext->GetGLTextureInfo(obj, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetGLTextureInfo(lua_State *L)
    {
    ud_t *ud;
    cl_mem gl_object = checkmemobject(L, 1, &ud);
    cl_gl_texture_info name = checkgltextureinfo(L, 2);
    if(!IsGLTexture(ud))
        return luaL_argerror(L, 1, "not a gl texture");
    CheckExtPfn(L, ud, GetGLTextureInfo);

    switch(name)
        {
        case CL_GL_TEXTURE_TARGET: return GetTextureInfoTarget(L, gl_object, ud, name);
        case CL_GL_MIPMAP_LEVEL: return GetTextureInfoInt(L, gl_object, ud, name);
        case CL_GL_NUM_SAMPLES: return GetTextureInfoSize(L, gl_object, ud, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/

static int GetGLContextDevice(lua_State *L, cl_context_properties *properties, ud_t *ud, cl_gl_context_info name)
    {
    cl_device value;
    cl_int ec = ud->clext->GetGLContextInfoKHR(properties, name, sizeof(value), &value, NULL);
    Free(L, properties);
    CheckError(L, ec);
//  printf("0x%.8x\n", value);
    pushdevice(L, value);
    return 1;
    }


static int GetGLContextDevices(lua_State *L, cl_context_properties * properties, ud_t *ud, cl_gl_context_info name)
    {
    cl_int ec;
    cl_device *value;
    size_t size, n, i;

    ec = ud->clext->GetGLContextInfoKHR(properties, name, 0, NULL, &size);
    if(ec)
        {
        Free(L, properties);
        CheckError(L, ec);
        return 0;
        }

    lua_newtable(L);
    if(size == 0)
        {
        Free(L, properties);
        return 1;
        }

    n = size / sizeof(cl_device);
    value = (cl_device*)MallocNoErr(L, n*sizeof(cl_device));
    if(!value)
        {
        Free(L, properties);
        return luaL_error(L, errstring(ERR_MEMORY));
        }
    ec = ud->clext->GetGLContextInfoKHR(properties, name, sizeof(value), &value, NULL);
    Free(L, properties);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }
//  printf("0x%.8x %d\n", value[0], n);

    for(i=0; i < n; i++)
        {
        if(value[i]==0) continue;
        pushdevice(L, value[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, value);
    return 1;
    }

static int GetGLContextInfo(lua_State *L)
    {
    int err;
    ud_t *ud;
    cl_context_properties *properties;
    cl_platform platform = checkplatform(L, 1, &ud);
    cl_gl_context_info name = checkglcontextinfo(L, 3);
    CheckExtPfn(L, ud, GetGLContextInfoKHR);
    properties = echeckcontextproperties(L, 2, &err, platform);
    if(err < 0)
        return luaL_argerror(L, 2, lua_tostring(L, -1));

    switch(name)
        {
        // The called function must Free properties.
        case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR: 
            return GetGLContextDevice(L, properties, ud, name);
        case CL_DEVICES_FOR_GL_CONTEXT_KHR: 
            return GetGLContextDevices(L, properties, ud, name);
        default:
            Free(L, properties);
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/

static int CreateEventFromGLsync(lua_State *L)
    {
    cl_int ec;
    ud_t *ud;
    cl_event event;
    cl_context context = checkcontext(L, 1, &ud);
    cl_GLsync sync = (cl_GLsync)checklightuserdata(L, 2);
    CheckExtPfn(L, ud, CreateEventFromGLsyncKHR);
    event = ud->clext->CreateEventFromGLsyncKHR(context, sync, &ec);
    CheckError(L, ec);
    return newevent(L, context, event);
    }

/*---------------------------------------------------------------------------*/

static const struct luaL_Reg Functions[] = 
    {
//      { "create_from_gl_buffer", CreateFromGLBuffer }, -> buffer.c
//      { "create_from_gl_texture", CreateFromGLTexture }, --> image.c
//      { "create_from_gl_renderbuffer", CreateFromGLRenderbuffer }, -> image.c
        { "get_gl_object_type", GetGLObjectType },
        { "get_gl_object_name", GetGLObjectName },
        { "get_gl_texture_info", GetGLTextureInfo },
        { "get_gl_context_info", GetGLContextInfo },
        { "create_event_from_gl_sync", CreateEventFromGLsync },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_gl_object(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }

