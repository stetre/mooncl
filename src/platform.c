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

static int freeplatform(lua_State *L, ud_t *ud)
    {
    freechildren(L, CONTEXT_MT, ud);
    freechildren(L, DEVICE_MT, ud);
    if(ud->clext)
        {
        Free(L, ud->clext);
        ud->clext = NULL;
        }
    if(!freeuserdata(L, ud, "platform")) return 0;
    return 0;
    }

static int newplatform(lua_State *L, cl_platform platform)
    {
    ud_t *ud;
    ud = newuserdata(L, platform, PLATFORM_MT, "platform");
    ud->platform = platform;
    ud->device = NULL;
    ud->parent_ud = NULL;
    ud->clext = mooncl_getproc_extensions(L, platform);
    ud->destructor = freeplatform;
    return 1;
    }

static int GetPlatforms(lua_State *L)
    {
    cl_int ec;
    cl_platform *platforms;
    cl_uint num_platforms, i;
    
    ec = cl.GetPlatformIDs(0, NULL, &num_platforms);
    CheckError(L, ec);

    if(num_platforms == 0)
        CheckError(L, CL_PLATFORM_NOT_FOUND_KHR); // just to be sure...

    platforms = (cl_platform*)Malloc(L, num_platforms * sizeof(cl_platform));
    ec = cl.GetPlatformIDs(num_platforms, platforms, NULL);
    if(ec)
        {
        Free(L, platforms);
        CheckError(L, ec);
        return 0;
        }

    lua_newtable(L);
    for(i = 0; i < num_platforms; i++)
        {
        if(!userdata(platforms[i]))
            newplatform(L, platforms[i]);
        else
            pushplatform(L, platforms[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, platforms);
    return 1;
    }



GET_INFO_STRING(GetString, Platform, platform, platform)
GET_INFO_ULONG(GetUlong, Platform, platform, platform)

static int GetPlatformInfo(lua_State *L)
    {
    cl_platform platform = checkplatform(L, 1, NULL);
    cl_platform_info name = checkplatforminfo(L, 2);
    switch(name)
        {
        case CL_PLATFORM_PROFILE: 
        case CL_PLATFORM_VERSION: 
        case CL_PLATFORM_NAME: 
        case CL_PLATFORM_VENDOR: 
        case CL_PLATFORM_EXTENSIONS: return GetString(L, platform, name);
        case CL_PLATFORM_HOST_TIMER_RESOLUTION: return GetUlong(L, platform, name); //CL_VERSION_2_1
//@@    case CL_PLATFORM_ICD_SUFFIX_KHR: 
// cl_int clIcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms);
        default:
            return unexpected(L);
        }
    return 0;
    }


RAW_FUNC(platform)
TYPE_FUNC(platform)
PARENT_FUNC(platform)
DELETE_FUNC(platform)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "parent", Parent },
        { "delete",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Delete },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "get_platform_ids", GetPlatforms },
        { "get_platform_info", GetPlatformInfo },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_platform(lua_State *L)
    {
    udata_define(L, PLATFORM_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

