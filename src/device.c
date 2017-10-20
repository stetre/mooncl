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

static int freedevice(lua_State *L, ud_t *ud)
    {
    cl_device device = (cl_device)ud->handle;
    if(HasSubDevices(ud))
        freechildren(L, DEVICE_MT, ud);
    if(!freeuserdata(L, ud, "device")) return 0;
    ReleaseAll(Device, DEVICE, device);
    return 0;
    }

static int newdevice(lua_State *L, cl_platform platform, cl_device device)
    {
    ud_t *ud;
    ud = newuserdata(L, device, DEVICE_MT, "device");
    ud->platform = platform;
    ud->device = device;
    ud->parent_ud = userdata(platform);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freedevice;
    return 1;
    }

static int newsubdevice(lua_State *L, cl_device parent, cl_device device)
    {
    ud_t *ud;
    ud_t *parent_ud = UD(parent);
    ud = newuserdata(L, device, DEVICE_MT, "sub device");
    ud->platform = parent_ud->platform;
    ud->device = device;
    ud->parent_ud = parent_ud;
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freedevice;
    MarkSubDevice(ud);
    MarkHasSubDevices(parent_ud);
    return 1;
    }

int createcontextdevices(lua_State *L, cl_platform platform, cl_context context)
/* Retrieves the context's devices and creates the binding object for those that
 * do not have it already.
 */
    {
    cl_int ec;
    cl_device *value;
    size_t size, n, i;

    ec = cl.GetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &size);
    CheckError(L, ec);

    if(size == 0) return 0;

    n = size / sizeof(cl_device);
    value = (cl_device*)Malloc(L, n*sizeof(cl_device));
    ec = cl.GetContextInfo(context, CL_CONTEXT_DEVICES, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }

    for(i=0; i < n; i++)
        {
        if(value[i]==0) continue;
        if(userdata(value[i])) continue; /* already created */
        newdevice(L, platform, value[i]);
        lua_pop(L, 1);
        }
    Free(L, value);
    return 0;
    }

static int GetDevices(lua_State *L)
    {
    cl_int ec;
    cl_device *devices;
    cl_uint i, num_devices = 0;
    cl_platform platform = checkplatform(L, 1, NULL);
    cl_device_type device_type = checkflags(L, 2);

    ec = cl.GetDeviceIDs(platform, device_type, 0, NULL, &num_devices);
    if(ec)
        CheckError(L, ec);

    if(num_devices == 0)
        CheckError(L, CL_DEVICE_NOT_FOUND); // paranoid ...

    lua_newtable(L);
    devices = (cl_device*)Malloc(L, num_devices * sizeof(cl_device));
    ec = cl.GetDeviceIDs(platform, device_type, num_devices, devices, NULL);
    if(ec)
        {
        Free(L, devices);
        CheckError(L, ec);
        return 0;
        }

    for(i = 0; i < num_devices; i++)
        {
        if(!userdata(devices[i]))
            newdevice(L, platform, devices[i]);
        else
            pushdevice(L, devices[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, devices);
    return 1;
    }

static int CreateSubDevices(lua_State *L)
    {
    int err;
    cl_int ec;
    cl_uint num_devices, i;
    cl_device *out_devices;

    cl_device in_device = checkdevice(L, 1, NULL);
    cl_device_partition_property *properties = echeckdevicepartitionproperty(L, 2, &err);
    if(err)
        return luaL_argerror(L, 2, lua_tostring(L, -1));

    /* get the number of devices this partition scheme will create */
    ec = cl.CreateSubDevices(in_device, properties, 0, NULL, &num_devices);
    if(ec)
        {
        Free(L, properties);
        CheckError(L, ec);
        return 0;
        }

    out_devices = (cl_device*)MallocNoErr(L, num_devices * sizeof(cl_device));
    if(!out_devices)
        {
        Free(L, properties);
        return luaL_error(L, errstring(ERR_MEMORY));
        }

    ec = cl.CreateSubDevices(in_device, properties, num_devices, out_devices, NULL);
    Free(L, properties);

    if(ec)
        {
        Free(L, out_devices);
        CheckError(L, ec);
        return 0;
        }
    lua_newtable(L);
    for(i = 0; i < num_devices; i++)
        {
        newsubdevice(L, in_device, out_devices[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, out_devices);
    return 1;
    }

static int SetDefaultDeviceCommandQueue(lua_State *L)
    {
    cl_int ec;
    cl_context context = checkcontext(L, 1, NULL);
    cl_device device = checkdevice(L, 2, NULL);
    cl_queue queue = checkqueue(L, 3, NULL);
    CheckPfn_2_1(L, SetDefaultDeviceCommandQueue);
    ec = cl.SetDefaultDeviceCommandQueue(context, device, queue);
    CheckError(L, ec);
    return 0;
    }


GET_INFO_STRING(GetString, Device, device, device)
GET_INFO_UINT(GetUint, Device, device, device)
GET_INFO_ULONG(GetUlong, Device, device, device)
GET_INFO_SIZE(GetSize, Device, device, device)
GET_INFO_FLAGS(GetFlags, Device, device, device)
GET_INFO_BOOLEAN(GetBoolean, Device, device, device)

GET_INFO_ENUM(GetMemCacheType, Device, device, device, pushdevicememcachetype)
GET_INFO_ENUM(GetLocalMemType, Device, device, device, pushdevicelocalmemtype)
GET_INFO_OBJECT(GetPlatform, Device, device, device, platform)
GET_INFO_OBJECT(GetDevice, Device, device, device, device)

static int GetMaxWorkItemSizes(lua_State *L, cl_device obj)
    {
    cl_int ec;
    size_t *value;
    cl_uint n, i;

    ec = cl.GetDeviceInfo(obj, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(n), &n, NULL);
    CheckError(L, ec);

    lua_newtable(L);
    if(n==0)
        return 1;
    
    value = (size_t*)Malloc(L, n*sizeof(size_t));
    ec = cl.GetDeviceInfo(obj, CL_DEVICE_MAX_WORK_ITEM_SIZES, n*sizeof(size_t), value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }

    for(i = 0; i < n; i++)
        {
        lua_pushinteger(L, value[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, value);
    return 1;
    }

static int GetPartitionProperties(lua_State *L, cl_device obj)
    {
    cl_int ec;
    cl_device_partition_property *value;
    size_t size, n, i;

    ec = cl.GetDeviceInfo(obj, CL_DEVICE_PARTITION_PROPERTIES, 0, NULL, &size);
    CheckError(L, ec);
    n = size / sizeof(cl_device_partition_property);
    value = (cl_device_partition_property*)Malloc(L, n*sizeof(cl_device_partition_property));
    ec = cl.GetDeviceInfo(obj, CL_DEVICE_PARTITION_PROPERTIES, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }

    lua_newtable(L);
    /* pre set all fields to false */
    lua_pushboolean(L, 0); lua_setfield(L, -2, "equally");
    lua_pushboolean(L, 0); lua_setfield(L, -2, "by_affinity_domain");
    lua_pushboolean(L, 0); lua_setfield(L, -2, "by_counts");
    for(i = 0; i < n; i++)
        {
        switch(value[i])
            {
            case CL_DEVICE_PARTITION_EQUALLY: 
                    lua_pushboolean(L, 1); lua_setfield(L, -2, "equally"); break;
            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN: 
                    lua_pushboolean(L, 1); lua_setfield(L, -2, "by_affinity_domain"); break;
            case CL_DEVICE_PARTITION_BY_COUNTS: 
                    lua_pushboolean(L, 1); lua_setfield(L, -2, "by_counts"); break;
            }
        }
    Free(L, value);
    return 1;
    }

static int GetPartitionType(lua_State *L, cl_device obj)
    {
    cl_int ec;
    cl_device_partition_property *value;
    size_t size, n;

    ec = cl.GetDeviceInfo(obj, CL_DEVICE_PARTITION_TYPE, 0, NULL, &size);
    CheckError(L, ec);

    if(size == 0) return 0;

    n = size / sizeof(cl_device_partition_property);
    value = (cl_device_partition_property*)Malloc(L, n*sizeof(cl_device_partition_property));
    ec = cl.GetDeviceInfo(obj, CL_DEVICE_PARTITION_TYPE, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }
    
    pushdevicepartitionproperty(L, value);
    Free(L, value);
    return 1;
    }

static int GetDeviceInfo(lua_State *L)
    {
    cl_device device = checkdevice(L, 1, NULL);
    cl_device_info name = checkdeviceinfo(L, 2);
    switch(name)
        {
        case CL_DEVICE_TYPE:    return GetFlags(L, device, name);
        case CL_DEVICE_VENDOR_ID:   return GetUint(L, device, name);
        case CL_DEVICE_MAX_COMPUTE_UNITS:   return GetUint(L, device, name);
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_WORK_ITEM_SIZES: return GetMaxWorkItemSizes(L, device);
        case CL_DEVICE_MAX_WORK_GROUP_SIZE: return GetSize(L, device, name);
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF: return GetUint(L, device, name);
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
        case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_CLOCK_FREQUENCY: return GetUint(L, device, name);
        case CL_DEVICE_ADDRESS_BITS:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:  return GetUlong(L, device, name);
        case CL_DEVICE_IMAGE_SUPPORT:   return GetBoolean(L, device, name);
        case CL_DEVICE_MAX_READ_IMAGE_ARGS:
        case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
        case CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS:   return GetUint(L, device, name);
        case CL_DEVICE_IMAGE2D_MAX_WIDTH:
        case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
        case CL_DEVICE_IMAGE3D_MAX_WIDTH:
        case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
        case CL_DEVICE_IMAGE3D_MAX_DEPTH:   return GetSize(L, device, name);
        case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
        case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:    return GetSize(L, device, name);
        case CL_DEVICE_MAX_SAMPLERS:    return GetUint(L, device, name);
        case CL_DEVICE_IMAGE_PITCH_ALIGNMENT:
        case CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_PIPE_ARGS:
        case CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS:
        case CL_DEVICE_PIPE_MAX_PACKET_SIZE:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_PARAMETER_SIZE:  return GetSize(L, device, name);
        case CL_DEVICE_MEM_BASE_ADDR_ALIGN: return GetUint(L, device, name);
        case CL_DEVICE_SINGLE_FP_CONFIG:    return GetFlags(L, device, name);
        case CL_DEVICE_DOUBLE_FP_CONFIG:    return GetFlags(L, device, name);
        case CL_DEVICE_HALF_FP_CONFIG:  return GetFlags(L, device, name);
        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:   return GetMemCacheType(L, device, name);
        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:   return GetUint(L, device, name);
        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:   return GetUlong(L, device, name);
        case CL_DEVICE_GLOBAL_MEM_SIZE: return GetUlong(L, device, name);
        case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:    return GetUlong(L, device, name);
        case CL_DEVICE_MAX_CONSTANT_ARGS:   return GetUint(L, device, name);
        case CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE:
        case CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE:    return GetSize(L, device, name);
        case CL_DEVICE_LOCAL_MEM_TYPE:  return GetLocalMemType(L, device, name);
        case CL_DEVICE_LOCAL_MEM_SIZE:  return GetUlong(L, device, name);
        case CL_DEVICE_ERROR_CORRECTION_SUPPORT:    return GetBoolean(L, device, name);
        case CL_DEVICE_PROFILING_TIMER_RESOLUTION:  return GetSize(L, device, name);
        case CL_DEVICE_ENDIAN_LITTLE:
        case CL_DEVICE_AVAILABLE:
        case CL_DEVICE_COMPILER_AVAILABLE:
        case CL_DEVICE_LINKER_AVAILABLE:    return GetBoolean(L, device, name);
        case CL_DEVICE_EXECUTION_CAPABILITIES:  return GetFlags(L, device, name);
//      case CL_DEVICE_QUEUE_PROPERTIES: DEPRECATED
        case CL_DEVICE_QUEUE_ON_HOST_PROPERTIES:    return GetFlags(L, device, name);
        case CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES:  return GetFlags(L, device, name);
        case CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE:  return GetUint(L, device, name);
        case CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_ON_DEVICE_QUEUES:    return GetUint(L, device, name);
        case CL_DEVICE_MAX_ON_DEVICE_EVENTS:    return GetUint(L, device, name);
        case CL_DEVICE_BUILT_IN_KERNELS:    return GetString(L, device, name);
        case CL_DEVICE_PLATFORM:    return GetPlatform(L, device, name);
        case CL_DEVICE_NAME:    return GetString(L, device, name);
        case CL_DEVICE_VENDOR:  return GetString(L, device, name);
        case CL_DRIVER_VERSION: return GetString(L, device, name);
        case CL_DEVICE_PROFILE: return GetString(L, device, name);
        case CL_DEVICE_VERSION: return GetString(L, device, name);
        case CL_DEVICE_OPENCL_C_VERSION:    return GetString(L, device, name);
        case CL_DEVICE_EXTENSIONS:  return GetString(L, device, name);
        case CL_DEVICE_PRINTF_BUFFER_SIZE:  return GetSize(L, device, name);
        case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC: return GetBoolean(L, device, name);
        case CL_DEVICE_PARENT_DEVICE:   return GetDevice(L, device, name);
        case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:   return GetUint(L, device, name);
        case CL_DEVICE_PARTITION_PROPERTIES:    return GetPartitionProperties(L, device);
        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:   return GetFlags(L, device, name);
        case CL_DEVICE_PARTITION_TYPE:  return GetPartitionType(L, device);
        case CL_DEVICE_REFERENCE_COUNT: return GetUint(L, device, name);
        case CL_DEVICE_SVM_CAPABILITIES:    return GetFlags(L, device, name);
        case CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT:
        case CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT:
        case CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT:    return GetUint(L, device, name);
        case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:    return GetUint(L, device, name);
//      case CL_DEVICE_HOST_UNIFIED_MEMORY: return Get(L, device, name); DEPRECATED
        case CL_DEVICE_IL_VERSION:  return GetString(L, device, name);//CL_VERSION_2_1
        case CL_DEVICE_MAX_NUM_SUB_GROUPS:  return GetUint(L, device, name);//CL_VERSION_2_1
        case CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS:  return GetBoolean(L, device, name);//CL_VERSION_2_1
//@@    case CL_DEVICE_TERMINATE_CAPABILITY_KHR:
//@@    case CL_DEVICE_SPIR_VERSIONS:
//@@    case CL_DEVICE_MAX_NAMED_BARRIER_COUNT_KHR: //VERSION_2_2 
        default:
            return unexpected(L);
        }
    return 0;
    }

static int GetDeviceAndHostTimer(lua_State *L)
    {
    cl_ulong device_timestamp, host_timestamp;
    cl_int ec;
    cl_device device = checkdevice(L, 1, NULL);
    CheckPfn_2_1(L, GetDeviceAndHostTimer);
    ec = cl.GetDeviceAndHostTimer(device, &device_timestamp, &host_timestamp);
    CheckError(L, ec);
    lua_pushinteger(L, device_timestamp);
    lua_pushinteger(L, host_timestamp);
    return 2;
    }

static int GetHostTimer(lua_State *L)
    {
    cl_ulong host_timestamp;
    cl_int ec;
    cl_device device = checkdevice(L, 1, NULL);
    CheckPfn_2_1(L, GetHostTimer);
    ec = cl.GetHostTimer(device, &host_timestamp);
    CheckError(L, ec);
    lua_pushinteger(L, host_timestamp);
    return 1;
    }

static int CheckEndianness(lua_State *L)
    {
    ud_t *ud;
    cl_int ec;
    cl_bool value;
    cl_device device = checkdevice(L, 1, &ud);
    ec = cl.GetDeviceInfo(device, CL_DEVICE_ENDIAN_LITTLE, sizeof(value), &value, NULL);
    CheckError(L, ec);
    value = value ? 1 : 0;
    lua_pushboolean(L, value == is_little_endian());
    lua_pushstring(L, value ? "little" : "big");
    return 2;
    }

RAW_FUNC(device)
TYPE_FUNC(device)
PLATFORM_FUNC(device)
PARENT_FUNC(device)
DELETE_FUNC(device)
RETAIN_FUNC(device, Device)
RELEASE_FUNC(device, Device, DEVICE)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "platform", Platform },
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
        { "get_device_ids", GetDevices },
        { "get_device_info", GetDeviceInfo },
        { "create_sub_devices", CreateSubDevices },
        { "retain_device", Retain },
        { "release_device", Release },
        { "set_default_device_command_queue", SetDefaultDeviceCommandQueue },
        { "get_device_and_host_timer", GetDeviceAndHostTimer },
        { "get_host_timer", GetHostTimer },
        { "check_endianness", CheckEndianness },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_device(lua_State *L)
    {
    udata_define(L, DEVICE_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

