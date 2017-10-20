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

static int freeprogram(lua_State *L, ud_t *ud)
    {
    cl_program program = (cl_program)ud->handle;
    freechildren(L, KERNEL_MT, ud);
    if(!freeuserdata(L, ud, "program")) return 0;
    ReleaseAll(Program, PROGRAM, program);
    return 0;
    }

static int newprogram(lua_State *L, cl_context context, cl_program program)
    {
    ud_t *ud;
    ud = newuserdata(L, program, PROGRAM_MT, "program");
    ud->context = context;
    ud->parent_ud = UD(context);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freeprogram;  
    return 1;
    }


static int CreateProgramWithSource(lua_State *L)
    {
    cl_int ec;
    size_t length;
    cl_program program;

    cl_context context = checkcontext(L, 1, NULL);
    const char *source = luaL_checklstring(L, 2, &length);
    
    program = cl.CreateProgramWithSource(context, 1, &source, &length, &ec);
    CheckError(L, ec);
    newprogram(L, context, program);
    return 1;
    }

static int CreateProgramWithBinary(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint count, bin_count, i;
    cl_program program;
    size_t *lengths = NULL;
    cl_device *devices = NULL;
    char **binaries = NULL;
    cl_int *binary_status = NULL;

#define CLEANUP() do {                                  \
    if(devices) Free(L, devices);                       \
    if(binary_status) Free(L, binary_status);           \
    if(binaries)                                        \
        {                                               \
        freelstringlist(L, binaries, lengths, count);   \
        }                                               \
} while(0)

    cl_context context = checkcontext(L, 1, NULL);
    devices = checkdevicelist(L, 2, &count, &err);
    if(err) return luaL_argerror(L, 2, errstring(err));

    binary_status = (cl_int*)MallocNoErr(L, sizeof(cl_int)*count);
    if(!binary_status)
        { CLEANUP(); return luaL_error(L, errstring(ERR_MEMORY)); }
    
    binaries = checklstringlist(L, 3, &bin_count, &lengths, &err);
    if(err)
        { CLEANUP(); return luaL_argerror(L, 3, errstring(err)); }
    if(bin_count != count)
        { CLEANUP(); return luaL_argerror(L, 3, errstring(ERR_LENGTH)); }
    
    program = cl.CreateProgramWithBinary(context, count, devices, lengths, 
            (const unsigned char **)binaries, binary_status, &ec);

    if(ec == CL_INVALID_BINARY) /* return nil, {binary_status} */
        {
        lua_pushnil(L);
        lua_newtable(L);
        for(i=0; i<count; i++)
            {
            pusherrcode(L, binary_status[i]);
            lua_rawseti(L, -2, i+1);
            }
        CLEANUP();
        return 2;
        }

    CLEANUP();
    CheckError(L, ec);

    newprogram(L, context, program);
    return 1;
#undef CLEANUP
    }

static int CreateProgramWithBuiltInKernels(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint count;
    cl_program program;
    const char * names;

    cl_context context = checkcontext(L, 1, NULL);
    cl_device *devices = checkdevicelist(L, 2, &count, &err);
    if(err) return luaL_argerror(L, 2, errstring(err));
    names = luaL_checkstring(L, 3);

    program = cl.CreateProgramWithBuiltInKernels(context, count, devices, names, &ec);
    Free(L, devices);
    CheckError(L, ec);
    newprogram(L, context, program);
    return 1;
    }

static int CreateProgramWithIL(lua_State *L)
    {
    cl_int ec;
    size_t length;
    cl_program program;
    cl_context context = checkcontext(L, 1, NULL);
    const void* il = luaL_checklstring(L, 2, &length);
    
    CheckPfn_2_1(L, CreateProgramWithIL);
    program = cl.CreateProgramWithIL(context, il, length, &ec);

    CheckError(L, ec);
    newprogram(L, context, program);
    return 1;
    }

static int BuildProgram(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint num_devices;
    cl_device *devices;
    const char *options;

    cl_program program = checkprogram(L, 1, NULL);

    devices = checkdevicelist(L, 2, &num_devices, &err); /* optional */
    if(err < 0) return luaL_argerror(L, 2, errstring(err));

    options = luaL_optstring(L, 3, NULL);

    ec = cl.BuildProgram(program, num_devices, devices, options, NULL, NULL);
    Free(L, devices);

    if(ec)
        {
        lua_pushboolean(L, 0);
        pusherrcode(L, ec);
        return 2;
        }
    
    lua_pushboolean(L, 1);
    return 1;
    }

static int CompileProgram(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint num_devices, num_headers, num_names;
    cl_device *devices;
    const char *options;
    cl_program *headers;
    char **header_names;

    cl_program program = checkprogram(L, 1, NULL);

    devices = checkdevicelist(L, 2, &num_devices, &err); /* optional */
    if(err < 0) return luaL_argerror(L, 2, errstring(err));

    options = luaL_optstring(L, 3, NULL);

    headers = checkprogramlist(L, 4, &num_headers, &err);
    if(err < 0) 
        {
        Free(L, devices);
        return luaL_argerror(L, 4, errstring(err));
        }
    
    header_names = checkstringlist(L, 5, &num_names, &err);
    if(err < 0)
        {
        Free(L, headers);
        Free(L, devices);
        return luaL_argerror(L, 5, errstring(err));
        }

    if(num_names != num_headers)
        {
        Free(L, headers);
        Free(L, devices);
        freestringlist(L, header_names, num_names);
        return luaL_argerror(L, 5, "headers list and names list have different lengths");
        }

    ec = cl.CompileProgram(program, num_devices, devices, options, num_headers, headers, 
                (const char **)header_names, NULL /*pfn_notify*/, NULL /*user_data */);
    Free(L, headers);
    Free(L, devices);
    freestringlist(L, header_names, num_names);

    if(ec)
        {
        lua_pushboolean(L, 0);
        pusherrcode(L, ec);
        return 2;
        }
    
    lua_pushboolean(L, 1);
    return 1;
    }


static int LinkProgram(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint num_devices, num_programs;
    cl_device *devices;
    const char *options;
    cl_program *programs, program;

    cl_context context = checkcontext(L, 1, NULL);

    devices = checkdevicelist(L, 2, &num_devices, &err); /* optional */
    if(err < 0) return luaL_argerror(L, 2, errstring(err));

    options = luaL_optstring(L, 3, NULL);

    programs = checkprogramlist(L, 4, &num_programs, &err);
    if(err < 0) 
        {
        Free(L, devices);
        return luaL_argerror(L, 4, errstring(err));
        }
    
    program = cl.LinkProgram(context, num_devices, devices, options, num_programs, programs, 
                NULL /*pfn_notify*/, NULL /*user_data */, &ec);
    Free(L, programs);
    Free(L, devices);
    if(ec)
        {
        lua_pushnil(L);
        pusherrcode(L, ec);
        return 2;
        }

    newprogram(L, context, program);
    return 1;
    }


static int UnloadPlatformCompiler(lua_State *L)
    {
    cl_platform platform = checkplatform(L, 1, NULL);
    cl_int ec = cl.UnloadPlatformCompiler(platform);
    CheckError(L, ec);
    return 0;
    }


/*---------------------------------------------------------------------------*/
GET_INFO_UINT(GetUint, Program, program, program)
GET_INFO_OBJECT(GetContext, Program, program, program, context)
GET_INFO_DEVICES(GetDevices, Program, program, program)
GET_INFO_STRING(GetString, Program, program, program)
GET_INFO_SIZE(GetSize, Program, program, program)
GET_INFO_BOOLEAN(GetBoolean, Program, program, program)

static int GetBinarySizes(lua_State *L, cl_program obj)
    {
    cl_int ec;
    size_t *sizes;
    cl_uint num_devices, i;

    ec = cl.GetProgramInfo(obj, CL_PROGRAM_NUM_DEVICES, sizeof(num_devices), &num_devices, NULL);
    CheckError(L, ec);

    if(num_devices == 0)
        { lua_newtable(L); return 1; }

    sizes = (size_t*)Malloc(L, num_devices*sizeof(size_t));

    ec = cl.GetProgramInfo(obj, CL_PROGRAM_BINARY_SIZES, num_devices*sizeof(size_t), sizes, NULL);
    if(ec)
        {
        Free(L, sizes);
        CheckError(L, ec);
        return 0;
        }

    lua_newtable(L);
    
    for(i=0; i < num_devices; i++)
        {
        lua_pushinteger(L, sizes[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, sizes);
    return 1;
    }


static int GetBinaries(lua_State *L, cl_program obj)
    {
    cl_int ec;
    size_t *sizes, total_size;
    char **binaries = NULL;
    char *bin;
    cl_uint num_devices, i;

    ec = cl.GetProgramInfo(obj, CL_PROGRAM_NUM_DEVICES, sizeof(num_devices), &num_devices, NULL);
    CheckError(L, ec);

    if(num_devices == 0)
        { lua_newtable(L); return 1; }

    sizes = (size_t*)Malloc(L, num_devices*sizeof(size_t));
    binaries = (char**)MallocNoErr(L, num_devices*sizeof(char*));
    if(!binaries) 
        {
        Free(L, sizes);
        return luaL_error(L, errstring(ERR_MEMORY));
        }

    ec = cl.GetProgramInfo(obj, CL_PROGRAM_BINARY_SIZES, num_devices*sizeof(size_t), sizes, NULL);
    if(ec)
        {
        Free(L, sizes);
        Free(L, binaries);
        CheckError(L, ec);
        return 0;
        }

    total_size = 0;
    for(i=0; i<num_devices; i++) total_size += sizes[i];

    bin = (char*)MallocNoErr(L, total_size);
    if(!bin)
        {
        Free(L, sizes);
        Free(L, binaries);
        return luaL_error(L, errstring(ERR_MEMORY));
        }

    binaries[0] = bin;
    for(i=1; i<num_devices; i++)
        bin[i] = bin[i-1] + sizes[i-1];

    ec = cl.GetProgramInfo(obj, CL_PROGRAM_BINARIES, num_devices*sizeof(char*), binaries, NULL);
    if(ec)
        {
        Free(L, sizes);
        Free(L, binaries);
        Free(L, bin);
        CheckError(L, ec);
        return 0;
        }
    
    lua_newtable(L);
    
    for(i=0; i < num_devices; i++)
        {
        if(sizes[i]==0)
            lua_pushstring(L, "");
        else
            lua_pushlstring(L, binaries[i], sizes[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, sizes);
    Free(L, binaries);
    Free(L, bin);
    return 1;
    }


static int GetProgramInfo(lua_State *L)
    {
    cl_program program = checkprogram(L, 1, NULL);
    cl_program_info name = checkprograminfo(L, 2);
    switch(name)
        {
        case CL_PROGRAM_REFERENCE_COUNT: return GetUint(L, program, name);
        case CL_PROGRAM_CONTEXT: return GetContext(L, program, name);
        case CL_PROGRAM_NUM_DEVICES: return GetUint(L, program, name);
        case CL_PROGRAM_DEVICES: return GetDevices(L, program, name);
        case CL_PROGRAM_SOURCE: return GetString(L, program, name);
        case CL_PROGRAM_BINARY_SIZES: return GetBinarySizes(L, program);
        case CL_PROGRAM_BINARIES: return GetBinaries(L, program);
        case CL_PROGRAM_NUM_KERNELS: return GetSize(L, program, name);
        case CL_PROGRAM_KERNEL_NAMES: return GetString(L, program, name);
        case CL_PROGRAM_IL: return GetString(L, program, name); //CL_VERSION_2_1
        case CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT: return GetBoolean(L, program, name); //CL_VERSION_2_2
        case CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT: return GetBoolean(L, program, name); //CL_VERSION_2_2
        default:
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/
#define GET_PROGRAM_BUILD_INFO_ENUM(Func, pushfunc)                             \
static int Func(lua_State *L, cl_program program, cl_device device, cl_program_build_info name) \
    {                                                                           \
    cl_uint value;                                                              \
    cl_int ec = cl.GetProgramBuildInfo(program, device, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                          \
    pushfunc(L, value);                                                         \
    return 1;                                                                   \
    }

GET_PROGRAM_BUILD_INFO_ENUM(GetProgramBuildStatus, pushbuildstatus)
GET_PROGRAM_BUILD_INFO_ENUM(GetProgramBinaryType, pushprogrambinarytype)

static int GetProgramBuildString(lua_State *L, cl_program program, cl_device device, cl_program_build_info name)
    {
    cl_int ec;
    char *value;
    size_t size;

    ec = cl.GetProgramBuildInfo(program, device, name, 0, NULL, &size);
    CheckError(L, ec);

    if(size==0)
        { lua_pushstring(L, ""); return 1; }
    value = (char*)Malloc(L, size);
    ec = cl.GetProgramBuildInfo(program, device, name, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }
    lua_pushlstring(L, value, size);
    Free(L, value);
    return 1;
    }

static int GetProgramBuildSize(lua_State *L, cl_program program, cl_device device, cl_program_build_info name)
    {
    size_t value;
    cl_int ec = cl.GetProgramBuildInfo(program, device, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetProgramBuildInfo(lua_State *L)
    {
    cl_program program = checkprogram(L, 1, NULL);
    cl_device device = checkdevice(L, 2, NULL);
    cl_program_build_info name = checkprogrambuildinfo(L, 3);
    switch(name)
        {
        case CL_PROGRAM_BUILD_STATUS: return GetProgramBuildStatus(L, program, device, name);
        case CL_PROGRAM_BINARY_TYPE: return GetProgramBinaryType(L, program, device, name);
        case CL_PROGRAM_BUILD_OPTIONS:
        case CL_PROGRAM_BUILD_LOG: return GetProgramBuildString(L, program, device, name);
        case CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE: 
                    return GetProgramBuildSize(L, program, device, name);
        default:
            return unexpected(L);
        }
    return 0;
    }



/*---------------------------------------------------------------------------*/

#if 0 
//@@ cl_int cl.SetProgramReleaseCallback(cl_program program, void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data), void *user_data); CL_VERSION_2_2
        { "",  },
static int (lua_State *L)
    {
    cl_program program = checkprogram(L, 1, NULL);
    (void)program;
    CheckPfn_2_2(L, SetProgramReleaseCallback);
    return 0;
    }

#endif

// cl_int cl.SetProgramSpecializationConstant(cl_program program, cl_uint spec_id, size_t spec_size, const void* spec_value);
static int SetProgramSpecializationConstant(lua_State *L) //@@
    {
    cl_program program = checkprogram(L, 1, NULL);
    (void)program;
    //return 0;
    CheckPfn_2_2(L, SetProgramSpecializationConstant);
    return notavailable(L);
    }

/*---------------------------------------------------------------------------*/

RAW_FUNC(program)
TYPE_FUNC(program)
PARENT_FUNC(program)
CONTEXT_FUNC(program)
DELETE_FUNC(program)
RETAIN_FUNC(program, Program)
RELEASE_FUNC(program, Program, PROGRAM)


static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "parent", Parent },
        { "context", Context },
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
        { "create_program_with_source", CreateProgramWithSource },
        { "create_program_with_binary", CreateProgramWithBinary },
        { "create_program_with_built_in_kernels", CreateProgramWithBuiltInKernels },
        { "create_program_with_il", CreateProgramWithIL },
        { "retain_program", Retain },
        { "release_program", Release },
        { "build_program_", BuildProgram },
        { "compile_program_", CompileProgram },
        { "link_program_", LinkProgram },
        { "unload_platform_compiler", UnloadPlatformCompiler },
        { "get_program_info", GetProgramInfo },
        { "get_program_build_info", GetProgramBuildInfo },
        { "set_program_specialization_constant", SetProgramSpecializationConstant },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_program(lua_State *L)
    {
    udata_define(L, PROGRAM_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

