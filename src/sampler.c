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

static int freesampler(lua_State *L, ud_t *ud)
    {
    cl_sampler sampler = (cl_sampler)ud->handle;
    if(!freeuserdata(L, ud, "sampler")) return 0;
    ReleaseAll(Sampler, SAMPLER, sampler);
    return 0;
    }

static int newsampler(lua_State *L, cl_context context, cl_sampler sampler)
    {
    ud_t *ud;
    ud = newuserdata(L, sampler, SAMPLER_MT, "sampler");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freesampler;  
    return 1;
    }

static int CreateSamplerWithProperties(lua_State *L)
    {
    cl_int ec;
    int err;
    cl_sampler sampler;
    cl_bool v1_2_normalized_coords;
    cl_addressing_mode v1_2_addressing_mode;
    cl_filter_mode v1_2_filter_mode;
    cl_context context = checkcontext(L, 1, NULL);
    cl_sampler_properties *properties = echecksamplerproperties(L, 2,
        &v1_2_normalized_coords, &v1_2_addressing_mode, &v1_2_filter_mode, &err);
    if(err < 0)
        return luaL_argerror(L, 2, errstring(err));

    //@@CheckPfn_2_0(L, CreateSamplerWithProperties);
    if(cl.CreateSamplerWithProperties) /* >= VERSION_2_0 */
        sampler = cl.CreateSamplerWithProperties(context, properties, &ec);
    else /* VERSION_1_2 */
        sampler = cl.CreateSampler(context, v1_2_normalized_coords, 
                v1_2_addressing_mode, v1_2_filter_mode, &ec);
    Free(L, properties);
    CheckError(L, ec);
    newsampler(L, context, sampler);
    return 1;
    }

GET_INFO_UINT(GetUint, Sampler, sampler, sampler)
GET_INFO_BOOLEAN(GetBoolean, Sampler, sampler, sampler)
GET_INFO_OBJECT(GetContext, Sampler, sampler, sampler, context )
GET_INFO_ENUM(GetAddressingMode, Sampler, sampler, sampler, pushaddressingmode)
GET_INFO_ENUM(GetFilterMode, Sampler, sampler, sampler, pushfiltermode)
GET_INFO_FLOAT(GetFloat, Sampler, sampler, sampler)

static int GetSamplerInfo(lua_State *L)
    {
    cl_sampler sampler = checksampler(L, 1, NULL);
    cl_sampler_info name = checksamplerinfo(L, 2);
    switch(name)
        {
        case CL_SAMPLER_REFERENCE_COUNT: return GetUint(L, sampler, name);
        case CL_SAMPLER_CONTEXT: return GetContext(L, sampler, name);
        case CL_SAMPLER_NORMALIZED_COORDS: return GetBoolean(L, sampler, name);
        case CL_SAMPLER_ADDRESSING_MODE: return GetAddressingMode(L, sampler, name);
        case CL_SAMPLER_FILTER_MODE: return GetFilterMode(L, sampler, name);
        case CL_SAMPLER_MIP_FILTER_MODE: return GetFilterMode(L, sampler, name);
        case CL_SAMPLER_LOD_MIN: return GetFloat(L, sampler, name);
        case CL_SAMPLER_LOD_MAX: return GetFloat(L, sampler, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

 
RAW_FUNC(sampler)
TYPE_FUNC(sampler)
CONTEXT_FUNC(sampler)
PARENT_FUNC(sampler)
DELETE_FUNC(sampler)
RETAIN_FUNC(sampler, Sampler)
RELEASE_FUNC(sampler, Sampler, SAMPLER)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "context", Context },
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
        { "create_sampler", CreateSamplerWithProperties },
        { "retain_sampler", Retain },
        { "release_sampler", Release },
        { "get_sampler_info", GetSamplerInfo },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_sampler(lua_State *L)
    {
    udata_define(L, SAMPLER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

