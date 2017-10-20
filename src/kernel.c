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

static int freekernel(lua_State *L, ud_t *ud)
    {
    cl_kernel kernel = (cl_kernel)ud->handle;
    if(!freeuserdata(L, ud, "kernel")) return 0;
    ReleaseAll(Kernel, KERNEL, kernel);
    return 0;
    }

static int newkernel(lua_State *L, cl_program program, cl_kernel kernel)
    {
    ud_t *ud;

    ud = newuserdata(L, kernel, KERNEL_MT, "kernel");
    ud->program = program;
    ud->parent_ud = UD(program);
    ud->clext = ud->parent_ud->clext;
    ud->destructor = freekernel;  
    return 1;
    }

static int CreateKernel(lua_State *L)
    {
    cl_int ec;
    cl_program program = checkprogram(L, 1, NULL);
    const char *kernel_name = luaL_checkstring(L, 2);
    cl_kernel kernel = cl.CreateKernel(program, kernel_name, &ec);
    CheckError(L, ec);
    newkernel(L, program, kernel);
    return 1;
    }

static int CreateKernelsInProgram(lua_State *L)
    {
    cl_int ec;
    cl_uint count, i;
    cl_kernel *kernels;
    
    cl_program program = checkprogram(L, 1, NULL);

    ec = cl.CreateKernelsInProgram(program, 0, NULL, &count);
    CheckError(L, ec);

    if(count==0)
        { lua_newtable(L); return 1; }

    kernels = (cl_kernel*)Malloc(L, count*sizeof(cl_kernel));
    ec = cl.CreateKernelsInProgram(program, count, kernels, NULL);
    if(ec)
        {
        Free(L, kernels);
        CheckError(L, ec);
        return 0;
        }
    
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        newkernel(L, program, kernels[i]);
        lua_rawseti(L, -2, i+1);
        }
    Free(L, kernels);
    return 1;   
    }

static int CloneKernel(lua_State *L)
    {
    cl_int ec;  
    ud_t *ud;
    cl_kernel source_kernel = checkkernel(L, 1, &ud);
    cl_kernel kernel;
    CheckPfn_2_1(L, CloneKernel);
    kernel = cl.CloneKernel(source_kernel, &ec);
    CheckError(L, ec);
    newkernel(L, ud->program, kernel);
    return 1;
    }

static int SetKernelArg(lua_State *L)
    {
    cl_int ec;
    int t, err, type, n;
    void *object;
    cl_char value_char;
    cl_uchar value_uchar;
    cl_short value_short;
    cl_ushort value_ushort;
    cl_int value_int;
    cl_uint value_uint;
    cl_long value_long;
    cl_ulong value_ulong;
    cl_half value_half;
    cl_float value_float;
    cl_double value_double;
    size_t arg_size = 0;
    const void *arg_value = NULL;
    void *data = NULL;

    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_uint arg_index = luaL_checkinteger(L, 2);

    t = lua_type(L, 3);

    if(t == LUA_TUSERDATA)
        {
        object = testmemobject(L, 3, NULL);
        if(!object) object = testsampler(L, 3, NULL);
        if(!object) object = testqueue(L, 3, NULL);
        if(!object)
            return luaL_argerror(L, 3, errstring(ERR_TYPE));
        arg_value = &object;
        arg_size = sizeof(object);
        }
    else if(t == LUA_TNUMBER)
        {
        arg_size = luaL_checkinteger(L, 3);
        arg_value = optlightuserdata(L, 4);
        }
    else if(t == LUA_TNIL)
        {
        arg_value = luaL_checklstring(L, 4, &arg_size);
        }
    else /* primitive type (scalar or vector) */
        {
        type = checkprimtype(L, 3);

        if(lua_type(L, 4) == LUA_TTABLE || !lua_isnoneornil(L, 5)) 
            { /* vector */
            n = toflattable(L, 4);
            arg_size = n * sizeofprimtype(type);
            data = Malloc(L, arg_size);
            err = testdata(L, type, n, data, arg_size);
            if(err) 
                { 
                Free(L, data); 
                return luaL_argerror(L, 4, errstring(err));
                }
            arg_value = data;
            }
        else /* scalar */
            {
            switch(type)
                {
#define CHECK(t, int_or_num) do {                           \
                value_##t = luaL_check##int_or_num(L, 4);   \
                arg_value = &value_##t;                     \
                arg_size = sizeof(cl_##t);                  \
            } while(0)
                case NONCL_TYPE_CHAR:   CHECK(char, integer); break;
                case NONCL_TYPE_UCHAR:  CHECK(uchar, integer); break;
                case NONCL_TYPE_SHORT:  CHECK(short, integer); break;
                case NONCL_TYPE_USHORT: CHECK(ushort, integer); break;
                case NONCL_TYPE_INT:    CHECK(int, integer); break;
                case NONCL_TYPE_UINT:   CHECK(uint, integer); break;
                case NONCL_TYPE_LONG:   CHECK(long, integer); break;
                case NONCL_TYPE_ULONG:  CHECK(ulong, integer); break;
                case NONCL_TYPE_HALF:   CHECK(half, number); break;
                case NONCL_TYPE_FLOAT:  CHECK(float, number); break;
                case NONCL_TYPE_DOUBLE: CHECK(double, number); break;
                default: return unexpected(L);
#undef CHECK
                }
            }
        }
    ec = cl.SetKernelArg(kernel, arg_index, arg_size, arg_value);
    if(data) Free(L, data);
    CheckError(L, ec);
    return 0;
    }

static int SetKernelArgSVMPointer(lua_State *L)
    {
    cl_int ec;
    void *arg_value;
    size_t offset;
    cl_svm svm;
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_uint arg_indx = luaL_checkinteger(L, 2);

    CheckPfn_2_0(L, SetKernelArgSVMPointer);
    if(lua_type(L, 3) == LUA_TLIGHTUSERDATA)
        arg_value = lua_touserdata(L, 3);
    else
        {
        svm = checksvm(L, 3, NULL);
        offset = luaL_checkinteger(L, 4);
        arg_value = (char*)svm->ptr + offset;
        }
    ec = cl.SetKernelArgSVMPointer(kernel, arg_indx, arg_value);
    CheckError(L, ec);
    return 0;
    }

/*---------------------------------------------------------------------------*/
GET_INFO_STRING(GetString, Kernel, kernel, kernel)
GET_INFO_UINT(GetUint, Kernel, kernel, kernel)
GET_INFO_OBJECT(GetContext, Kernel, kernel, kernel, context)
GET_INFO_OBJECT(GetProgram, Kernel, kernel, kernel, program)
GET_INFO_SIZE(GetSize, Kernel, kernel, kernel)

static int GetKernelInfo(lua_State *L)
    {
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_kernel_info name = checkkernelinfo(L, 2);
    switch(name)
        {
        case CL_KERNEL_FUNCTION_NAME: return GetString(L, kernel, name);
        case CL_KERNEL_NUM_ARGS: return GetUint(L, kernel, name);
        case CL_KERNEL_REFERENCE_COUNT: return GetUint(L, kernel, name);
        case CL_KERNEL_CONTEXT: return GetContext(L, kernel, name);
        case CL_KERNEL_PROGRAM: return GetProgram(L, kernel, name);
        case CL_KERNEL_ATTRIBUTES: return GetString(L, kernel, name);
        case CL_KERNEL_MAX_NUM_SUB_GROUPS: return GetSize(L, kernel, name); //CL_VERSION_2_1
        case CL_KERNEL_COMPILE_NUM_SUB_GROUPS: return GetSize(L, kernel, name); //CL_VERSION_2_1
        default:
            return unexpected(L);
        }
    return 0;
    }


/*---------------------------------------------------------------------------*/

static int SetSvmPtrs(lua_State *L, cl_kernel kernel, cl_kernel_exec_info name)
    {
    cl_int ec;
    int err;
    cl_uint count;
    void **val = checklightuserdatalist(L, 3, &count, &err);
    if(err)
        return luaL_argerror(L, 3, errstring(err));

    ec = cl.SetKernelExecInfo(kernel, name, count * sizeof(void*), val);
    Free(L, val);
    CheckError(L, ec);
    return 0;
    }

static int SetKernelExecInfoBoolean(lua_State *L, cl_kernel kernel, cl_kernel_exec_info name)
    {
    cl_bool val = checkboolean(L, 3);
    cl_int ec = cl.SetKernelExecInfo(kernel, name, sizeof(val), &val);
    CheckError(L, ec);
    return 0;
    }

static int SetKernelExecInfo(lua_State *L)
    {
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_kernel_exec_info name = checkkernelexecinfo(L, 2);

    CheckPfn_2_0(L, SetKernelExecInfo);

    switch(name)
        {
        case CL_KERNEL_EXEC_INFO_SVM_PTRS: return SetSvmPtrs(L, kernel, name);
        case CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM: return SetKernelExecInfoBoolean(L, kernel, name);
        default:
            return unexpected(L);
        }
    return 0;
    }
/*---------------------------------------------------------------------------*/
#define GET_KERNEL_ARG_INFO_ENUM(Func, pushfunc)                                \
static int Func(lua_State *L, cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info name)  \
    {                                                                           \
    cl_uint value;                                                              \
    cl_int ec = cl.GetKernelArgInfo(kernel, arg_indx, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                          \
    pushfunc(L, value);                                                         \
    return 1;                                                                   \
    }

GET_KERNEL_ARG_INFO_ENUM(GetKernelArgAddressQualifier, pushargaddressqualifier)
GET_KERNEL_ARG_INFO_ENUM(GetKernelArgAccessQualifier, pushargaccessqualifier)

static int GetKernelArgFlags(lua_State *L, cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info name)
    {
    cl_bitfield value;
    cl_int ec = cl.GetKernelArgInfo(kernel, arg_indx, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetKernelArgString(lua_State *L, cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info name)
    {
    cl_int ec;
    char *value;
    size_t size;

    ec = cl.GetKernelArgInfo(kernel, arg_indx, name, 0, NULL, &size);
    CheckError(L, ec);

    if(size==0)
        { lua_pushstring(L, ""); return 1; }

    value = (char*)Malloc(L, size);
    ec = cl.GetKernelArgInfo(kernel, arg_indx, name, size, value, NULL);
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

static int GetKernelArgInfo(lua_State *L)
    {
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_uint arg_indx = luaL_checkinteger(L, 2);
    cl_kernel_arg_info name = checkkernelarginfo(L, 3);
    switch(name)
        {
        case CL_KERNEL_ARG_ADDRESS_QUALIFIER: 
                return GetKernelArgAddressQualifier(L, kernel, arg_indx, name);
        case CL_KERNEL_ARG_ACCESS_QUALIFIER:
                return GetKernelArgAccessQualifier(L, kernel, arg_indx, name);
        case CL_KERNEL_ARG_TYPE_NAME: return GetKernelArgString(L, kernel, arg_indx, name);
        case CL_KERNEL_ARG_TYPE_QUALIFIER: return GetKernelArgFlags(L, kernel, arg_indx, name);
        case CL_KERNEL_ARG_NAME: return GetKernelArgString(L, kernel, arg_indx, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/

static int GetKernelWorkGroupUlong(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_work_group_info name)
    {
    cl_ulong value;
    cl_int ec = cl.GetKernelWorkGroupInfo(kernel, device, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetKernelWorkGroupSize(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_work_group_info name)
    {
    size_t value;
    cl_int ec = cl.GetKernelWorkGroupInfo(kernel, device, name, sizeof(value), &value, NULL);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetKernelWorkGroupSize3(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_work_group_info name)
    {
    int i;
    size_t value[3];
    cl_int ec = cl.GetKernelWorkGroupInfo(kernel, device, name, 3*sizeof(size_t), value, NULL);
    CheckError(L, ec);
    lua_newtable(L);
    for(i=0; i<3; i++)
        {
        lua_pushinteger(L, value[i]);
        lua_rawseti(L, -2, i+1);
        }
    return 1;
    }

static int GetKernelWorkGroupInfo(lua_State *L)
    {
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_device device = checkdevice(L, 2, NULL);
    cl_kernel_work_group_info name = checkkernelworkgroupinfo(L, 3);
    switch(name)
        {
        case CL_KERNEL_GLOBAL_WORK_SIZE:
        case CL_KERNEL_COMPILE_WORK_GROUP_SIZE: 
                    return GetKernelWorkGroupSize3(L, kernel, device, name);
        case CL_KERNEL_WORK_GROUP_SIZE: 
        case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
                    return GetKernelWorkGroupSize(L, kernel, device, name);
        case CL_KERNEL_LOCAL_MEM_SIZE:
        case CL_KERNEL_PRIVATE_MEM_SIZE: 
                    return GetKernelWorkGroupUlong(L, kernel, device, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/

static int GetSizeList_Size(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_sub_group_info name)
    {
    int err;
    cl_int ec;
    cl_uint count;
    size_t *input_value, value;

    input_value = checksizelist(L, 4, &count, &err);
    if(err) return luaL_argerror(L, 4, errstring(err));
    
    ec = cl.GetKernelSubGroupInfo(kernel, device, name, 
        count*sizeof(size_t), input_value, sizeof(size_t), &value, NULL);
    Free(L, input_value);
    CheckError(L, ec);
    lua_pushinteger(L, value);
    return 1;
    }

static int GetSize_SizeList(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_sub_group_info name)
    {
    cl_int ec;
    size_t size, input_value, *value = NULL;

    input_value = luaL_checkinteger(L, 4);
    
    ec = cl.GetKernelSubGroupInfo(kernel, device, name, sizeof(size_t), &input_value, 0, NULL, &size);
    CheckError(L, ec);

    value = (size_t*)Malloc(L, size);

    ec = cl.GetKernelSubGroupInfo(kernel, device, name, sizeof(size_t), &input_value, size, value, NULL);
    if(ec)
        {
        Free(L, value);
        CheckError(L, ec);
        return 0;
        }
    pushsizelist(L, value, size / sizeof(size_t));
    Free(L, value);
    return 1;
    }


static int GetNil_Size(lua_State *L, cl_kernel kernel, cl_device device, cl_kernel_sub_group_info name)
    {
    size_t value;

    cl_int ec = cl.GetKernelSubGroupInfo(kernel, device, name, 0, NULL, sizeof(size_t), &value, NULL);
    CheckError(L, ec);

    lua_pushinteger(L, value);
    return 1;
    }

static int GetKernelSubGroupInfo(lua_State *L)
    {
    cl_kernel kernel = checkkernel(L, 1, NULL);
    cl_device device = testdevice(L, 2, NULL);
    cl_kernel_sub_group_info name = checkkernelsubgroupinfo(L, 3);

    CheckPfn_2_1(L, GetKernelSubGroupInfo);

    switch(name)
        {
        case CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE: 
        case CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE: 
                        return GetSizeList_Size(L, kernel, device, name);
        case CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT: 
                        return GetSize_SizeList(L, kernel, device, name);
        case CL_KERNEL_MAX_NUM_SUB_GROUPS:
        case CL_KERNEL_COMPILE_NUM_SUB_GROUPS:
                        return GetNil_Size(L, kernel, device, name);
        default:
            return unexpected(L);
        }
    return 0;
    }

/*---------------------------------------------------------------------------*/

RAW_FUNC(kernel)
TYPE_FUNC(kernel)
PROGRAM_FUNC(kernel)
PARENT_FUNC(kernel)
DELETE_FUNC(kernel)
RETAIN_FUNC(kernel, Kernel)
RELEASE_FUNC(kernel, Kernel, KERNEL)

static const struct luaL_Reg Methods[] = 
    {
        { "raw", Raw },
        { "type", Type },
        { "program", Program },
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
        { "create_kernel", CreateKernel },
        { "create_kernels_in_program", CreateKernelsInProgram },
        { "clone_kernel", CloneKernel },
        { "retain_kernel", Retain },
        { "release_kernel", Release },
        { "set_kernel_arg", SetKernelArg },
        { "set_kernel_arg_svm_pointer", SetKernelArgSVMPointer },
        { "set_kernel_exec_info", SetKernelExecInfo },
        { "get_kernel_info", GetKernelInfo },
        { "get_kernel_arg_info", GetKernelArgInfo },
        { "get_kernel_work_group_info", GetKernelWorkGroupInfo },
        { "get_kernel_sub_group_info", GetKernelSubGroupInfo },
        { NULL, NULL } /* sentinel */
    };

void mooncl_open_kernel(lua_State *L)
    {
    udata_define(L, KERNEL_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

