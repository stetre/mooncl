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

static int freeimage(lua_State *L, ud_t *ud)
    {
    cl_image image = (cl_image)ud->handle;
    if(!freeuserdata(L, ud, "image")) return 0;
    ReleaseAll(MemObject, MEM, image);
    return 0;
    }

static void DestructorCallback(cl_image image, void *luastate)
    {
#define L (lua_State*)luastate
    ud_t *ud = UD(image);
    if(!ud) return; /* already deleted */
    ud->destructor(L, ud);  
#undef L
    }

static ud_t *newimage(lua_State *L, cl_context context, cl_image image)
    {
    ud_t *ud;
    ud = newuserdata(L, image, IMAGE_MT, "image");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freeimage;  
    cl.SetMemObjectDestructorCallback(image, DestructorCallback, L);
    return ud;
    }

static int CreateImage(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_image image;
    cl_image_format format;
    cl_image_desc desc;
    size_t size;
    void *host_ptr;

    cl_context context = checkcontext(L, 1, NULL);
    cl_mem_flags flags = checkflags(L, 2);
    err = echeckimageformat(L, 3, &format);
    if(err) return luaL_argerror(L, 3, lua_tostring(L, -1));
    err = echeckimagedesc(L, 4, &desc);
    if(err) return luaL_argerror(L, 4, lua_tostring(L, -1));
    
    if(lua_type(L, 5) == LUA_TSTRING)
        {
        if(flags & CL_MEM_USE_HOST_PTR)
            return luaL_argerror(L, 5, "the 'use host ptr' flag requires a pointer to valid memory");
        host_ptr = (void*)luaL_checklstring(L, 5, &size);
        if(size == 0)
            return luaL_argerror(L, 5, "zero length initializing data");
        }
    else
        {
        host_ptr = optlightuserdata(L, 5);
        }

    image = cl.CreateImage(context, flags, &format, &desc, host_ptr, &ec);
    CheckError(L, ec);
    newimage(L, context, image);
    return 1;
    }

static int CreateFromGLTexture(lua_State *L)
    {
    ud_t *ud, *context_ud;
    cl_int ec;
    cl_image image;
    cl_context context = checkcontext(L, 1, &context_ud);
    cl_mem_flags flags = checkflags(L, 2);
    cl_GLenum target = checkgltexturetarget(L, 3);
    cl_GLint miplevel = luaL_checkinteger(L, 4);
    cl_GLuint texture = luaL_checkinteger(L, 5);
    CheckExtPfn(L, context_ud, CreateFromGLTexture);
    image = context_ud->clext->CreateFromGLTexture(context, flags, target, miplevel, texture, &ec);
    CheckError(L, ec);
    ud = newimage(L, context, image);
    MarkGLTexture(ud);
    return 1;
    }

static int CreateFromGLRenderbuffer(lua_State *L)
    {
    ud_t *ud, *context_ud;
    cl_int ec;
    cl_image image;
    cl_context context = checkcontext(L, 1, &context_ud);
    cl_mem_flags flags = checkflags(L, 2);
    cl_GLuint renderbuffer = luaL_checkinteger(L, 3);
    CheckExtPfn(L, context_ud, CreateFromGLRenderbuffer);
    image = context_ud->clext->CreateFromGLRenderbuffer(context, flags, renderbuffer, &ec);
    CheckError(L, ec);
    ud = newimage(L, context, image);
    MarkGLRenderbuffer(ud);
    return 1;
    }

GET_INFO_STRUCT(GetImageFormat, Image, image, image, cl_image_format, pushimageformat)
GET_INFO_SIZE(GetSize, Image, image, image)
GET_INFO_UINT(GetUint, Image, image, image)

static int GetImageInfo(lua_State *L)
    {
    cl_image image = checkimage(L, 1, NULL);
    cl_image_info name = checkimageinfo(L, 2);
    switch(name)
        {
        case CL_IMAGE_FORMAT: return GetImageFormat(L, image, name);
        case CL_IMAGE_ELEMENT_SIZE: return GetSize(L, image, name);
        case CL_IMAGE_ROW_PITCH: return GetSize(L, image, name);
        case CL_IMAGE_SLICE_PITCH: return GetSize(L, image, name);
        case CL_IMAGE_WIDTH: return GetSize(L, image, name);
        case CL_IMAGE_HEIGHT: return GetSize(L, image, name);
        case CL_IMAGE_DEPTH: return GetSize(L, image, name);
        case CL_IMAGE_ARRAY_SIZE: return GetSize(L, image, name);
//      case CL_IMAGE_BUFFER: return GetSize(L, image, name); DEPRECATED
        case CL_IMAGE_NUM_MIP_LEVELS: return GetUint(L, image, name);
        case CL_IMAGE_NUM_SAMPLES: return GetUint(L, image, name);
        default:
            return unexpected(L);
        }
    return 0;
    }


RAW_FUNC(image)
TYPE_FUNC(image)
CONTEXT_FUNC(image)
PARENT_FUNC(image)
DELETE_FUNC(image)
RETAIN_FUNC(image, MemObject)
RELEASE_FUNC(image, MemObject, MEM)

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
        { "create_image", CreateImage },
        { "create_from_gl_texture", CreateFromGLTexture },
        { "create_from_gl_renderbuffer", CreateFromGLRenderbuffer },
        { "get_image_info", GetImageInfo },
        { "retain_image", Retain },
        { "release_image", Release },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_image(lua_State *L)
    {
    udata_define(L, IMAGE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

