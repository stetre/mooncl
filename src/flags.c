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

#define ADD(c) do { lua_pushinteger(L, CL_##c); lua_setfield(L, -2, #c); } while(0)

/* checkkkflags: accepts a list of strings starting from index=arg
 * pushxxxflags -> pushes a list of strings 
 */


#if 0
/*----------------------------------------------------------------------*
 | Reserved
 *----------------------------------------------------------------------*/

static cl_bitfield checkreservedflags(lua_State *L, int arg) 
    {
    const char *s;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        }

    return 0;
    }

static int pushreservedflags(lua_State *L, cl_bitfield flags)
    {
    (void)L; (void)flags;
    return 0;
    }


static int ReservedFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushreservedflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkreservedflags(L, 1));
    return 1;
    }
#endif


/*----------------------------------------------------------------------*
 | cl_device_type
 *----------------------------------------------------------------------*/

static cl_bitfield checkdevicetype(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_DEVICE_TYPE_DEFAULT, "default");
        CASE(CL_DEVICE_TYPE_CPU, "cpu");
        CASE(CL_DEVICE_TYPE_GPU, "gpu");
        CASE(CL_DEVICE_TYPE_ACCELERATOR, "accelerator");
        CASE(CL_DEVICE_TYPE_CUSTOM, "custom");
        CASE(CL_DEVICE_TYPE_ALL, "all");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdevicetype(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_DEVICE_TYPE_DEFAULT, "default");
        CASE(CL_DEVICE_TYPE_CPU, "cpu");
        CASE(CL_DEVICE_TYPE_GPU, "gpu");
        CASE(CL_DEVICE_TYPE_ACCELERATOR, "accelerator");
        CASE(CL_DEVICE_TYPE_CUSTOM, "custom");
//      CASE(CL_DEVICE_TYPE_ALL, "all");
#undef CASE

    return n;
    }

static int DeviceType(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdevicetype(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdevicetype(L, 1));
    return 1;
    }

#define Add_DeviceType(L)   \
        ADD(DEVICE_TYPE_DEFAULT);   \
        ADD(DEVICE_TYPE_CPU);       \
        ADD(DEVICE_TYPE_GPU);       \
        ADD(DEVICE_TYPE_ACCELERATOR);   \
        ADD(DEVICE_TYPE_CUSTOM);        \
        ADD(DEVICE_TYPE_ALL);\


/*----------------------------------------------------------------------*
 | cl_device_fp_config  
 *----------------------------------------------------------------------*/

static cl_bitfield checkdevicefpconfig(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_FP_DENORM, "denorm");
        CASE(CL_FP_INF_NAN, "inf nan");
        CASE(CL_FP_ROUND_TO_NEAREST, "round to nearest");
        CASE(CL_FP_ROUND_TO_ZERO, "round to zero");
        CASE(CL_FP_ROUND_TO_INF, "round to inf");
        CASE(CL_FP_FMA, "fma");
        CASE(CL_FP_SOFT_FLOAT, "float");
        CASE(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT, "correctly rounded divide sqrt");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdevicefpconfig(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_FP_DENORM, "denorm");
        CASE(CL_FP_INF_NAN, "inf nan");
        CASE(CL_FP_ROUND_TO_NEAREST, "round to nearest");
        CASE(CL_FP_ROUND_TO_ZERO, "round to zero");
        CASE(CL_FP_ROUND_TO_INF, "round to inf");
        CASE(CL_FP_FMA, "fma");
        CASE(CL_FP_SOFT_FLOAT, "float");
        CASE(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT, "correctly rounded divide sqrt");
#undef CASE

    return n;
    }

static int DeviceFpConfig(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdevicefpconfig(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdevicefpconfig(L, 1));
    return 1;
    }

#define Add_DeviceFpConfig(L) \
        ADD(FP_DENORM); \
        ADD(FP_INF_NAN);\
        ADD(FP_ROUND_TO_NEAREST);\
        ADD(FP_ROUND_TO_ZERO);\
        ADD(FP_ROUND_TO_INF);\
        ADD(FP_FMA);\
        ADD(FP_SOFT_FLOAT);\
        ADD(FP_CORRECTLY_ROUNDED_DIVIDE_SQRT);


/*----------------------------------------------------------------------*
 | cl_device_exec_capabilities
 *----------------------------------------------------------------------*/

static cl_bitfield checkdeviceexeccapabilities(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_EXEC_KERNEL, "kernel");
        CASE(CL_EXEC_NATIVE_KERNEL, "native kernel");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdeviceexeccapabilities(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_EXEC_KERNEL, "kernel");
        CASE(CL_EXEC_NATIVE_KERNEL, "native kernel");
#undef CASE

    return n;
    }

static int DeviceExecCapabilities(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdeviceexeccapabilities(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdeviceexeccapabilities(L, 1));
    return 1;
    }

#define Add_DeviceExecCapabilities(L) \
        ADD(EXEC_KERNEL);\
        ADD(EXEC_NATIVE_KERNEL);


/*----------------------------------------------------------------------*
 | cl_command_queue_properties 
 *----------------------------------------------------------------------*/

static cl_bitfield checkcommandqueueproperties(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, "out of order exec mode enable");
        CASE(CL_QUEUE_PROFILING_ENABLE, "profiling enable");
        CASE(CL_QUEUE_ON_DEVICE, "on device");
        CASE(CL_QUEUE_ON_DEVICE_DEFAULT, "on device default");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushcommandqueueproperties(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, "out of order exec mode enable");
        CASE(CL_QUEUE_PROFILING_ENABLE, "profiling enable");
        CASE(CL_QUEUE_ON_DEVICE, "on device");
        CASE(CL_QUEUE_ON_DEVICE_DEFAULT, "on device default");
#undef CASE

    return n;
    }

static int CommandQueueProperties(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushcommandqueueproperties(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkcommandqueueproperties(L, 1));
    return 1;
    }

#define Add_CommandQueueProperties(L) \
        ADD(QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);\
        ADD(QUEUE_PROFILING_ENABLE);\
        ADD(QUEUE_ON_DEVICE);\
        ADD(QUEUE_ON_DEVICE_DEFAULT);


/*----------------------------------------------------------------------*
 | cl_device_svm_capabilities  
 *----------------------------------------------------------------------*/

static cl_bitfield checkdevicesvmcapabilities(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER, "coarse grain buffer");
        CASE(CL_DEVICE_SVM_FINE_GRAIN_BUFFER, "fine grain buffer");
        CASE(CL_DEVICE_SVM_FINE_GRAIN_SYSTEM, "fine grain system");
        CASE(CL_DEVICE_SVM_ATOMICS, "atomics");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdevicesvmcapabilities(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER, "coarse grain buffer");
        CASE(CL_DEVICE_SVM_FINE_GRAIN_BUFFER, "fine grain buffer");
        CASE(CL_DEVICE_SVM_FINE_GRAIN_SYSTEM, "fine grain system");
        CASE(CL_DEVICE_SVM_ATOMICS, "atomics");
#undef CASE

    return n;
    }

static int DeviceSvmCapabilities(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdevicesvmcapabilities(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdevicesvmcapabilities(L, 1));
    return 1;
    }

#define Add_DeviceSvmCapabilities(L) \
        ADD(DEVICE_SVM_COARSE_GRAIN_BUFFER);\
        ADD(DEVICE_SVM_FINE_GRAIN_BUFFER);\
        ADD(DEVICE_SVM_FINE_GRAIN_SYSTEM);\
        ADD(DEVICE_SVM_ATOMICS);


/*----------------------------------------------------------------------*
 | cl_device_affinity_domain
 *----------------------------------------------------------------------*/

static cl_bitfield checkdeviceaffinitydomain(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_DEVICE_AFFINITY_DOMAIN_NUMA, "numa");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE, "l4 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE, "l3 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE, "l2 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE, "l1 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE, "next partitionable");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushdeviceaffinitydomain(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_DEVICE_AFFINITY_DOMAIN_NUMA, "numa");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE, "l4 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE, "l3 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE, "l2 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE, "l1 cache");
        CASE(CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE, "next partitionable");
#undef CASE

    return n;
    }

static int DeviceAffinityDomain(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushdeviceaffinitydomain(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkdeviceaffinitydomain(L, 1));
    return 1;
    }

#define Add_DeviceAffinityDomain(L) \
        ADD(DEVICE_AFFINITY_DOMAIN_NUMA);\
        ADD(DEVICE_AFFINITY_DOMAIN_L4_CACHE);\
        ADD(DEVICE_AFFINITY_DOMAIN_L3_CACHE);\
        ADD(DEVICE_AFFINITY_DOMAIN_L2_CACHE);\
        ADD(DEVICE_AFFINITY_DOMAIN_L1_CACHE);\
        ADD(DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE);



/*----------------------------------------------------------------------*
 | cl_mem_flags and cl_svm_mem_flags
 *----------------------------------------------------------------------*/

static cl_bitfield checkmemflags(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_MEM_READ_WRITE, "read write");
        CASE(CL_MEM_WRITE_ONLY, "write only");
        CASE(CL_MEM_READ_ONLY, "read only");
        CASE(CL_MEM_USE_HOST_PTR, "use host ptr");
        CASE(CL_MEM_ALLOC_HOST_PTR, "alloc host ptr");
        CASE(CL_MEM_COPY_HOST_PTR, "copy host ptr");
        CASE(CL_MEM_HOST_WRITE_ONLY, "host write only");
        CASE(CL_MEM_HOST_READ_ONLY, "host read only");
        CASE(CL_MEM_HOST_NO_ACCESS, "host no access");
        CASE(CL_MEM_SVM_FINE_GRAIN_BUFFER, "fine grain buffer");
        CASE(CL_MEM_SVM_ATOMICS, "atomics");
        CASE(CL_MEM_KERNEL_READ_AND_WRITE, "kernel read and write");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmemflags(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_MEM_READ_WRITE, "read write");
        CASE(CL_MEM_WRITE_ONLY, "write only");
        CASE(CL_MEM_READ_ONLY, "read only");
        CASE(CL_MEM_USE_HOST_PTR, "use host ptr");
        CASE(CL_MEM_ALLOC_HOST_PTR, "alloc host ptr");
        CASE(CL_MEM_COPY_HOST_PTR, "copy host ptr");
        CASE(CL_MEM_HOST_WRITE_ONLY, "host write only");
        CASE(CL_MEM_HOST_READ_ONLY, "host read only");
        CASE(CL_MEM_HOST_NO_ACCESS, "host no access");
        CASE(CL_MEM_SVM_FINE_GRAIN_BUFFER, "fine grain buffer");
        CASE(CL_MEM_SVM_ATOMICS, "atomics");
        CASE(CL_MEM_KERNEL_READ_AND_WRITE, "kernel read and write");
#undef CASE

    return n;
    }

static int MemFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmemflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmemflags(L, 1));
    return 1;
    }

#define Add_MemFlags(L) \
        ADD(MEM_READ_WRITE);\
        ADD(MEM_WRITE_ONLY);\
        ADD(MEM_READ_ONLY);\
        ADD(MEM_USE_HOST_PTR);\
        ADD(MEM_ALLOC_HOST_PTR);\
        ADD(MEM_COPY_HOST_PTR);\
        ADD(MEM_HOST_WRITE_ONLY);\
        ADD(MEM_HOST_READ_ONLY);\
        ADD(MEM_HOST_NO_ACCESS);\
        ADD(MEM_SVM_FINE_GRAIN_BUFFER);\
        ADD(MEM_SVM_ATOMICS);\
        ADD(MEM_KERNEL_READ_AND_WRITE);


/*----------------------------------------------------------------------*
 | cl_mem_migration_flags  
 *----------------------------------------------------------------------*/

static cl_bitfield checkmemmigrationflags(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
    CASE(CL_MIGRATE_MEM_OBJECT_HOST, "host");
    CASE(CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, "content undefined");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmemmigrationflags(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(CL_MIGRATE_MEM_OBJECT_HOST, "host");
    CASE(CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED, "content undefined");
#undef CASE

    return n;
    }

static int MemMigrationFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmemmigrationflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmemmigrationflags(L, 1));
    return 1;
    }

#define Add_MemMigrationFlags(L) \
    ADD(MIGRATE_MEM_OBJECT_HOST);\
    ADD(MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);
   

/*----------------------------------------------------------------------*
 | cl_map_flags  
 *----------------------------------------------------------------------*/

static cl_bitfield checkmapflags(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_MAP_READ, "read");
        CASE(CL_MAP_WRITE, "write");
        CASE(CL_MAP_WRITE_INVALIDATE_REGION, "write invalidate region");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushmapflags(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_MAP_READ, "read");
        CASE(CL_MAP_WRITE, "write");
        CASE(CL_MAP_WRITE_INVALIDATE_REGION, "write invalidate region");
#undef CASE

    return n;
    }

static int MapFlags(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushmapflags(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkmapflags(L, 1));
    return 1;
    }

#define Add_MapFlags(L) \
        ADD(MAP_READ);\
        ADD(MAP_WRITE);\
        ADD(MAP_WRITE_INVALIDATE_REGION);


/*----------------------------------------------------------------------*
 | cl_kernel_arg_type_qualifier  
 *----------------------------------------------------------------------*/

static cl_bitfield checkkernelargtypequalifier(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_KERNEL_ARG_TYPE_NONE, "none");
        CASE(CL_KERNEL_ARG_TYPE_CONST, "const");
        CASE(CL_KERNEL_ARG_TYPE_RESTRICT, "restrict");
        CASE(CL_KERNEL_ARG_TYPE_VOLATILE, "volatile");
        CASE(CL_KERNEL_ARG_TYPE_PIPE, "pipe");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushkernelargtypequalifier(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

//  if(flags==CL_KERNEL_ARG_TYPE_NONE) { lua_pushstring(L, "none"); return 1; }

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
        CASE(CL_KERNEL_ARG_TYPE_CONST, "const");
        CASE(CL_KERNEL_ARG_TYPE_RESTRICT, "restrict");
        CASE(CL_KERNEL_ARG_TYPE_VOLATILE, "volatile");
        CASE(CL_KERNEL_ARG_TYPE_PIPE, "pipe");
#undef CASE

    return n;
    }

static int KernelArgTypeQualifier(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushkernelargtypequalifier(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkkernelargtypequalifier(L, 1));
    return 1;
    }

#define Add_KernelArgTypeQualifier(L) \
        ADD(KERNEL_ARG_TYPE_NONE);\
        ADD(KERNEL_ARG_TYPE_CONST);\
        ADD(KERNEL_ARG_TYPE_RESTRICT);\
        ADD(KERNEL_ARG_TYPE_VOLATILE);\
        ADD(KERNEL_ARG_TYPE_PIPE);




/*------------------------------------------------------------------------------*
 | Additional utilities                                                         |
 *------------------------------------------------------------------------------*/

static int AddConstants(lua_State *L) /* cl.XXX constants for CL_XXX values */
    {
    Add_DeviceType(L);
    Add_DeviceFpConfig(L);
    Add_DeviceExecCapabilities(L);
    Add_CommandQueueProperties(L);
    Add_DeviceSvmCapabilities(L);
    Add_DeviceAffinityDomain(L);
    Add_MemFlags(L);
    Add_MemMigrationFlags(L);
    Add_MapFlags(L);
    Add_KernelArgTypeQualifier(L);
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "devicetypeflags", DeviceType },
        { "fpflags", DeviceFpConfig },
        { "execflags", DeviceExecCapabilities },
        { "queueflags", CommandQueueProperties },
        { "svmflags", DeviceSvmCapabilities },
        { "affinitydomainflags", DeviceAffinityDomain },
        { "memflags", MemFlags },
        { "migrateflags", MemMigrationFlags },
        { "mapflags", MapFlags },
        { "argtypeflags", KernelArgTypeQualifier },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_flags(lua_State *L)
    {
    AddConstants(L);
    luaL_setfuncs(L, Functions, 0);
    }


#if 0 // scaffolding

/*----------------------------------------------------------------------*
 | cl_ 
 *----------------------------------------------------------------------*/

static cl_bitfield checkzzz(lua_State *L, int arg) 
    {
    const char *s;
    cl_bitfield flags = 0;
    
    while(lua_isstring(L, arg))
        {
        s = lua_tostring(L, arg++);
#define CASE(CODE,str) if((strcmp(s, str)==0)) do { flags |= CODE; goto done; } while(0)
        CASE(CL_ZZZ_, "");
#undef CASE
        return (cl_bitfield)luaL_argerror(L, --arg, badvalue(L,s));
        done: ;
        }

    return flags;
    }

static int pushzzz(lua_State *L, cl_bitfield flags)
    {
    int n = 0;

#define CASE(CODE,str) do { if( flags & CODE) { lua_pushstring(L, str); n++; } } while(0)
    CASE(CL_ZZZ_, "");
#undef CASE

    return n;
    }

static int Zzz(lua_State *L)
    {
    if(lua_type(L, 1) == LUA_TNUMBER)
        return pushzzz(L, luaL_checkinteger(L, 1));
    lua_pushinteger(L, checkzzz(L, 1));
    return 1;
    }

    Add_Zzz(L);
        { "zzz", Zzz },
#define Add_Zzz(L) \
    ADD(ZZZ_);\

#endif

