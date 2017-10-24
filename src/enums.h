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

#ifndef enumsDEFINED
#define enumsDEFINED

/* enums.c */
#define enums_free_all mooncl_enums_free_all
void enums_free_all(lua_State *L);
#define enums_test mooncl_enums_test
uint32_t enums_test(lua_State *L, uint32_t domain, int arg, int *err);
#define enums_check mooncl_enums_check
uint32_t enums_check(lua_State *L, uint32_t domain, int arg);
#define enums_push mooncl_enums_push
int enums_push(lua_State *L, uint32_t domain, uint32_t code);
#define enums_values mooncl_enums_values
int enums_values(lua_State *L, uint32_t domain);
#define enums_checklist mooncl_enums_checklist
uint32_t* enums_checklist(lua_State *L, uint32_t domain, int arg, uint32_t *count, int *err);
#define enums_freelist mooncl_enums_freelist
void enums_freelist(lua_State *L, uint32_t *list);


/* Enum domains */
#define DOMAIN_RESULT                           0
#define DOMAIN_PLATFORM_INFO                    1
#define DOMAIN_DEVICE_INFO                      2
#define DOMAIN_DEVICE_MEM_CACHE_TYPE            3
#define DOMAIN_DEVICE_LOCAL_MEM_TYPE            4
#define DOMAIN_CONTEXT_INFO                     5
#define DOMAIN_COMMAND_QUEUE_INFO               7
#define DOMAIN_CHANNEL_ORDER                    8
#define DOMAIN_CHANNEL_TYPE                     9
#define DOMAIN_MEM_OBJECT_TYPE                  10
#define DOMAIN_MEM_INFO                         11
#define DOMAIN_IMAGE_INFO                       12
#define DOMAIN_PIPE_INFO                        13
#define DOMAIN_ADDRESSING_MODE                  14
#define DOMAIN_FILTER_MODE                      15
#define DOMAIN_SAMPLER_INFO                     16
#define DOMAIN_PROGRAM_INFO                     17
#define DOMAIN_PROGRAM_BUILD_INFO               18
#define DOMAIN_PROGRAM_BINARY_TYPE              19
#define DOMAIN_KERNEL_INFO                      20
#define DOMAIN_KERNEL_ARG_INFO                  21
#define DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER     22
#define DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER      23
#define DOMAIN_KERNEL_WORK_GROUP_INFO           24
#define DOMAIN_KERNEL_SUB_GROUP_INFO            25
#define DOMAIN_KERNEL_EXEC_INFO                 26
#define DOMAIN_EVENT_INFO                       27
#define DOMAIN_COMMAND_TYPE                     28
#define DOMAIN_COMMAND_EXECUTION_STATUS         29
#define DOMAIN_BUFFER_CREATE_TYPE               30
#define DOMAIN_PROFILING_INFO                   31
#define DOMAIN_BUILD_STATUS                     32
#define DOMAIN_GL_TEXTURE_INFO                  33
#define DOMAIN_GL_CONTEXT_INFO                  34
#define DOMAIN_GL_TEXTURE_TARGET                35 /* OpenGL GL_TEXTURE_XXX */
#define DOMAIN_GL_OBJECT_TYPE                   36 /* OpenGL CL_GL_OBJECT_XXX */
#define DOMAIN_QUEUE_PRIORITY                   37
#define DOMAIN_QUEUE_THROTTLE                   38
/* NONCL additions */
#define DOMAIN_NONCL_TYPE                       101

/* Types for cl.sizeof() & friends */
#define NONCL_TYPE_CHAR         1
#define NONCL_TYPE_UCHAR        2
#define NONCL_TYPE_SHORT        3
#define NONCL_TYPE_USHORT       4
#define NONCL_TYPE_INT          5
#define NONCL_TYPE_UINT         6
#define NONCL_TYPE_LONG         7
#define NONCL_TYPE_ULONG        8
#define NONCL_TYPE_HALF         9
#define NONCL_TYPE_FLOAT        10
#define NONCL_TYPE_DOUBLE       11

/* DOMAIN_GL_TEXTURE_TARGET (from gl.h) */
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_3D 0x806F
#define GL_TEXTURE_1D_ARRAY 0x8C18
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE_BUFFER 0x8C2A
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_TEXTURE_RECTANGLE 0x84F5
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102

#define testprimtype(L, arg, err) enums_test((L), DOMAIN_NONCL_TYPE, (arg), (err))
#define checkprimtype(L, arg) enums_check((L), DOMAIN_NONCL_TYPE, (arg))
#define pushprimtype(L, val) enums_push((L), DOMAIN_NONCL_TYPE, (uint32_t)(val))
#define valuesprimtype(L) enums_values((L), DOMAIN_NONCL_TYPE)

#define testplatforminfo(L, arg, err) (cl_platform_info)enums_test((L), DOMAIN_PLATFORM_INFO, (arg), (err))
#define checkplatforminfo(L, arg) (cl_platform_info)enums_check((L), DOMAIN_PLATFORM_INFO, (arg))
#define pushplatforminfo(L, val) enums_push((L), DOMAIN_PLATFORM_INFO, (uint32_t)(val))
#define valuesplatforminfo(L) enums_values((L), DOMAIN_PLATFORM_INFO)

#define testdeviceinfo(L, arg, err) (cl_device_info)enums_test((L), DOMAIN_DEVICE_INFO, (arg), (err))
#define checkdeviceinfo(L, arg) (cl_device_info)enums_check((L), DOMAIN_DEVICE_INFO, (arg))
#define pushdeviceinfo(L, val) enums_push((L), DOMAIN_DEVICE_INFO, (uint32_t)(val))
#define valuesdeviceinfo(L) enums_values((L), DOMAIN_DEVICE_INFO)

#define testdevicememcachetype(L, arg, err) (cl_device_mem_cache_type)enums_test((L), DOMAIN_DEVICE_MEM_CACHE_TYPE, (arg), (err))
#define checkdevicememcachetype(L, arg) (cl_device_mem_cache_type)enums_check((L), DOMAIN_DEVICE_MEM_CACHE_TYPE, (arg))
#define pushdevicememcachetype(L, val) enums_push((L), DOMAIN_DEVICE_MEM_CACHE_TYPE, (uint32_t)(val))
#define valuesdevicememcachetype(L) enums_values((L), DOMAIN_DEVICE_MEM_CACHE_TYPE)

#define testdevicelocalmemtype(L, arg, err) (cl_device_local_mem_type)enums_test((L), DOMAIN_DEVICE_LOCAL_MEM_TYPE, (arg), (err))
#define checkdevicelocalmemtype(L, arg) (cl_device_local_mem_type)enums_check((L), DOMAIN_DEVICE_LOCAL_MEM_TYPE, (arg))
#define pushdevicelocalmemtype(L, val) enums_push((L), DOMAIN_DEVICE_LOCAL_MEM_TYPE, (uint32_t)(val))
#define valuesdevicelocalmemtype(L) enums_values((L), DOMAIN_DEVICE_LOCAL_MEM_TYPE)

#define testcontextinfo(L, arg, err) (cl_context_info)enums_test((L), DOMAIN_CONTEXT_INFO, (arg), (err))
#define checkcontextinfo(L, arg) (cl_context_info)enums_check((L), DOMAIN_CONTEXT_INFO, (arg))
#define pushcontextinfo(L, val) enums_push((L), DOMAIN_CONTEXT_INFO, (uint32_t)(val))
#define valuescontextinfo(L) enums_values((L), DOMAIN_CONTEXT_INFO)

#define testcommandqueueinfo(L, arg, err) (cl_command_queue_info)enums_test((L), DOMAIN_COMMAND_QUEUE_INFO, (arg), (err))
#define checkcommandqueueinfo(L, arg) (cl_command_queue_info)enums_check((L), DOMAIN_COMMAND_QUEUE_INFO, (arg))
#define pushcommandqueueinfo(L, val) enums_push((L), DOMAIN_COMMAND_QUEUE_INFO, (uint32_t)(val))
#define valuescommandqueueinfo(L) enums_values((L), DOMAIN_COMMAND_QUEUE_INFO)

#define testchannelorder(L, arg, err) (cl_channel_order)enums_test((L), DOMAIN_CHANNEL_ORDER, (arg), (err))
#define checkchannelorder(L, arg) (cl_channel_order)enums_check((L), DOMAIN_CHANNEL_ORDER, (arg))
#define pushchannelorder(L, val) enums_push((L), DOMAIN_CHANNEL_ORDER, (uint32_t)(val))
#define valueschannelorder(L) enums_values((L), DOMAIN_CHANNEL_ORDER)

#define testchanneltype(L, arg, err) (cl_channel_type)enums_test((L), DOMAIN_CHANNEL_TYPE, (arg), (err))
#define checkchanneltype(L, arg) (cl_channel_type)enums_check((L), DOMAIN_CHANNEL_TYPE, (arg))
#define pushchanneltype(L, val) enums_push((L), DOMAIN_CHANNEL_TYPE, (uint32_t)(val))
#define valueschanneltype(L) enums_values((L), DOMAIN_CHANNEL_TYPE)

#define testmemobjecttype(L, arg, err) (cl_mem_object_type)enums_test((L), DOMAIN_MEM_OBJECT_TYPE, (arg), (err))
#define checkmemobjecttype(L, arg) (cl_mem_object_type)enums_check((L), DOMAIN_MEM_OBJECT_TYPE, (arg))
#define pushmemobjecttype(L, val) enums_push((L), DOMAIN_MEM_OBJECT_TYPE, (uint32_t)(val))
#define valuesmemobjecttype(L) enums_values((L), DOMAIN_MEM_OBJECT_TYPE)

#define testmeminfo(L, arg, err) (cl_mem_info)enums_test((L), DOMAIN_MEM_INFO, (arg), (err))
#define checkmeminfo(L, arg) (cl_mem_info)enums_check((L), DOMAIN_MEM_INFO, (arg))
#define pushmeminfo(L, val) enums_push((L), DOMAIN_MEM_INFO, (uint32_t)(val))
#define valuesmeminfo(L) enums_values((L), DOMAIN_MEM_INFO)

#define testimageinfo(L, arg, err) (cl_image_info)enums_test((L), DOMAIN_IMAGE_INFO, (arg), (err))
#define checkimageinfo(L, arg) (cl_image_info)enums_check((L), DOMAIN_IMAGE_INFO, (arg))
#define pushimageinfo(L, val) enums_push((L), DOMAIN_IMAGE_INFO, (uint32_t)(val))
#define valuesimageinfo(L) enums_values((L), DOMAIN_IMAGE_INFO)

#define testpipeinfo(L, arg, err) (cl_pipe_info)enums_test((L), DOMAIN_PIPE_INFO, (arg), (err))
#define checkpipeinfo(L, arg) (cl_pipe_info)enums_check((L), DOMAIN_PIPE_INFO, (arg))
#define pushpipeinfo(L, val) enums_push((L), DOMAIN_PIPE_INFO, (uint32_t)(val))
#define valuespipeinfo(L) enums_values((L), DOMAIN_PIPE_INFO)

#define testaddressingmode(L, arg, err) (cl_addressing_mode)enums_test((L), DOMAIN_ADDRESSING_MODE, (arg), (err))
#define checkaddressingmode(L, arg) (cl_addressing_mode)enums_check((L), DOMAIN_ADDRESSING_MODE, (arg))
#define pushaddressingmode(L, val) enums_push((L), DOMAIN_ADDRESSING_MODE, (uint32_t)(val))
#define valuesaddressingmode(L) enums_values((L), DOMAIN_ADDRESSING_MODE)

#define testfiltermode(L, arg, err) (cl_filter_mode)enums_test((L), DOMAIN_FILTER_MODE, (arg), (err))
#define checkfiltermode(L, arg) (cl_filter_mode)enums_check((L), DOMAIN_FILTER_MODE, (arg))
#define pushfiltermode(L, val) enums_push((L), DOMAIN_FILTER_MODE, (uint32_t)(val))
#define valuesfiltermode(L) enums_values((L), DOMAIN_FILTER_MODE)

#define testsamplerinfo(L, arg, err) (cl_sampler_info)enums_test((L), DOMAIN_SAMPLER_INFO, (arg), (err))
#define checksamplerinfo(L, arg) (cl_sampler_info)enums_check((L), DOMAIN_SAMPLER_INFO, (arg))
#define pushsamplerinfo(L, val) enums_push((L), DOMAIN_SAMPLER_INFO, (uint32_t)(val))
#define valuessamplerinfo(L) enums_values((L), DOMAIN_SAMPLER_INFO)

#define testprograminfo(L, arg, err) (cl_program_info)enums_test((L), DOMAIN_PROGRAM_INFO, (arg), (err))
#define checkprograminfo(L, arg) (cl_program_info)enums_check((L), DOMAIN_PROGRAM_INFO, (arg))
#define pushprograminfo(L, val) enums_push((L), DOMAIN_PROGRAM_INFO, (uint32_t)(val))
#define valuesprograminfo(L) enums_values((L), DOMAIN_PROGRAM_INFO)

#define testprogrambuildinfo(L, arg, err) (cl_program_build_info)enums_test((L), DOMAIN_PROGRAM_BUILD_INFO, (arg), (err))
#define checkprogrambuildinfo(L, arg) (cl_program_build_info)enums_check((L), DOMAIN_PROGRAM_BUILD_INFO, (arg))
#define pushprogrambuildinfo(L, val) enums_push((L), DOMAIN_PROGRAM_BUILD_INFO, (uint32_t)(val))
#define valuesprogrambuildinfo(L) enums_values((L), DOMAIN_PROGRAM_BUILD_INFO)

#define testprogrambinarytype(L, arg, err) (cl_program_binary_type)enums_test((L), DOMAIN_PROGRAM_BINARY_TYPE, (arg), (err))
#define checkprogrambinarytype(L, arg) (cl_program_binary_type)enums_check((L), DOMAIN_PROGRAM_BINARY_TYPE, (arg))
#define pushprogrambinarytype(L, val) enums_push((L), DOMAIN_PROGRAM_BINARY_TYPE, (uint32_t)(val))
#define valuesprogrambinarytype(L) enums_values((L), DOMAIN_PROGRAM_BINARY_TYPE)

#define testkernelinfo(L, arg, err) (cl_kernel_info)enums_test((L), DOMAIN_KERNEL_INFO, (arg), (err))
#define checkkernelinfo(L, arg) (cl_kernel_info)enums_check((L), DOMAIN_KERNEL_INFO, (arg))
#define pushkernelinfo(L, val) enums_push((L), DOMAIN_KERNEL_INFO, (uint32_t)(val))
#define valueskernelinfo(L) enums_values((L), DOMAIN_KERNEL_INFO)

#define testkernelarginfo(L, arg, err) (cl_kernel_arg_info)enums_test((L), DOMAIN_KERNEL_ARG_INFO, (arg), (err))
#define checkkernelarginfo(L, arg) (cl_kernel_arg_info)enums_check((L), DOMAIN_KERNEL_ARG_INFO, (arg))
#define pushkernelarginfo(L, val) enums_push((L), DOMAIN_KERNEL_ARG_INFO, (uint32_t)(val))
#define valueskernelarginfo(L) enums_values((L), DOMAIN_KERNEL_ARG_INFO)

#define testargaddressqualifier(L, arg, err) (cl_kernel_arg_address_qualifier)enums_test((L), DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER, (arg), (err))
#define checkargaddressqualifier(L, arg) (cl_kernel_arg_address_qualifier)enums_check((L), DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER, (arg))
#define pushargaddressqualifier(L, val) enums_push((L), DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER, (uint32_t)(val))
#define valuesargaddressqualifier(L) enums_values((L), DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER)

#define testargaccessqualifier(L, arg, err) (cl_kernel_arg_access_qualifier)enums_test((L), DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER, (arg), (err))
#define checkargaccessqualifier(L, arg) (cl_kernel_arg_access_qualifier)enums_check((L), DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER, (arg))
#define pushargaccessqualifier(L, val) enums_push((L), DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER, (uint32_t)(val))
#define valuesargaccessqualifier(L) enums_values((L), DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER)

#define testkernelworkgroupinfo(L, arg, err) (cl_kernel_work_group_info)enums_test((L), DOMAIN_KERNEL_WORK_GROUP_INFO, (arg), (err))
#define checkkernelworkgroupinfo(L, arg) (cl_kernel_work_group_info)enums_check((L), DOMAIN_KERNEL_WORK_GROUP_INFO, (arg))
#define pushkernelworkgroupinfo(L, val) enums_push((L), DOMAIN_KERNEL_WORK_GROUP_INFO, (uint32_t)(val))
#define valueskernelworkgroupinfo(L) enums_values((L), DOMAIN_KERNEL_WORK_GROUP_INFO)

#define testkernelsubgroupinfo(L, arg, err) (cl_kernel_sub_group_info)enums_test((L), DOMAIN_KERNEL_SUB_GROUP_INFO, (arg), (err))
#define checkkernelsubgroupinfo(L, arg) (cl_kernel_sub_group_info)enums_check((L), DOMAIN_KERNEL_SUB_GROUP_INFO, (arg))
#define pushkernelsubgroupinfo(L, val) enums_push((L), DOMAIN_KERNEL_SUB_GROUP_INFO, (uint32_t)(val))
#define valueskernelsubgroupinfo(L) enums_values((L), DOMAIN_KERNEL_SUB_GROUP_INFO)

#define testkernelexecinfo(L, arg, err) (cl_kernel_exec_info)enums_test((L), DOMAIN_KERNEL_EXEC_INFO, (arg), (err))
#define checkkernelexecinfo(L, arg) (cl_kernel_exec_info)enums_check((L), DOMAIN_KERNEL_EXEC_INFO, (arg))
#define pushkernelexecinfo(L, val) enums_push((L), DOMAIN_KERNEL_EXEC_INFO, (uint32_t)(val))
#define valueskernelexecinfo(L) enums_values((L), DOMAIN_KERNEL_EXEC_INFO)

#define testeventinfo(L, arg, err) (cl_event_info)enums_test((L), DOMAIN_EVENT_INFO, (arg), (err))
#define checkeventinfo(L, arg) (cl_event_info)enums_check((L), DOMAIN_EVENT_INFO, (arg))
#define pusheventinfo(L, val) enums_push((L), DOMAIN_EVENT_INFO, (uint32_t)(val))
#define valueseventinfo(L) enums_values((L), DOMAIN_EVENT_INFO)

#define testcommandtype(L, arg, err) (cl_command_type)enums_test((L), DOMAIN_COMMAND_TYPE, (arg), (err))
#define checkcommandtype(L, arg) (cl_command_type)enums_check((L), DOMAIN_COMMAND_TYPE, (arg))
#define pushcommandtype(L, val) enums_push((L), DOMAIN_COMMAND_TYPE, (uint32_t)(val))
#define valuescommandtype(L) enums_values((L), DOMAIN_COMMAND_TYPE)

#define testexecutionstatus(L, arg, err) (cl_channel_order)enums_test((L), DOMAIN_COMMAND_EXECUTION_STATUS, (arg), (err))
#define checkexecutionstatus(L, arg) (cl_channel_order)enums_check((L), DOMAIN_COMMAND_EXECUTION_STATUS, (arg))
#define pushexecutionstatus(L, val) enums_push((L), DOMAIN_COMMAND_EXECUTION_STATUS, (uint32_t)(val))
#define valuesexecutionstatus(L) enums_values((L), DOMAIN_COMMAND_EXECUTION_STATUS)

#define testbuffercreatetype(L, arg, err) (cl_buffer_create_type)enums_test((L), DOMAIN_BUFFER_CREATE_TYPE, (arg), (err))
#define checkbuffercreatetype(L, arg) (cl_buffer_create_type)enums_check((L), DOMAIN_BUFFER_CREATE_TYPE, (arg))
#define pushbuffercreatetype(L, val) enums_push((L), DOMAIN_BUFFER_CREATE_TYPE, (uint32_t)(val))
#define valuesbuffercreatetype(L) enums_values((L), DOMAIN_BUFFER_CREATE_TYPE)

#define testprofilinginfo(L, arg, err) (cl_profiling_info)enums_test((L), DOMAIN_PROFILING_INFO, (arg), (err))
#define checkprofilinginfo(L, arg) (cl_profiling_info)enums_check((L), DOMAIN_PROFILING_INFO, (arg))
#define pushprofilinginfo(L, val) enums_push((L), DOMAIN_PROFILING_INFO, (uint32_t)(val))
#define valuesprofilinginfo(L) enums_values((L), DOMAIN_PROFILING_INFO)

#define testbuildstatus(L, arg, err) (cl_build_status)enums_test((L), DOMAIN_BUILD_STATUS, (arg), (err))
#define checkbuildstatus(L, arg) (cl_build_status)enums_check((L), DOMAIN_BUILD_STATUS, (arg))
#define pushbuildstatus(L, val) enums_push((L), DOMAIN_BUILD_STATUS, (uint32_t)(val))
#define valuesbuildstatus(L) enums_values((L), DOMAIN_BUILD_STATUS)

#define testgltextureinfo(L, arg, err) (cl_gl_texture_info)enums_test((L), DOMAIN_GL_TEXTURE_INFO, (arg), (err))
#define checkgltextureinfo(L, arg) (cl_gl_texture_info)enums_check((L), DOMAIN_GL_TEXTURE_INFO, (arg))
#define pushgltextureinfo(L, val) enums_push((L), DOMAIN_GL_TEXTURE_INFO, (uint32_t)(val))
#define valuesgltextureinfo(L) enums_values((L), DOMAIN_GL_TEXTURE_INFO)

#define testglcontextinfo(L, arg, err) (cl_gl_context_info)enums_test((L), DOMAIN_GL_CONTEXT_INFO, (arg), (err))
#define checkglcontextinfo(L, arg) (cl_gl_context_info)enums_check((L), DOMAIN_GL_CONTEXT_INFO, (arg))
#define pushglcontextinfo(L, val) enums_push((L), DOMAIN_GL_CONTEXT_INFO, (uint32_t)(val))
#define valuesglcontextinfo(L) enums_values((L), DOMAIN_GL_CONTEXT_INFO)

#define testqueuepriority(L, arg, err) (cl_queue_priority_khr)enums_test((L), DOMAIN_QUEUE_PRIORITY, (arg), (err))
#define checkqueuepriority(L, arg) (cl_queue_priority_khr)enums_check((L), DOMAIN_QUEUE_PRIORITY, (arg))
#define pushqueuepriority(L, val) enums_push((L), DOMAIN_QUEUE_PRIORITY, (uint32_t)(val))
#define valuesqueuepriority(L) enums_values((L), DOMAIN_QUEUE_PRIORITY)

#define testqueuethrottle(L, arg, err) (cl_queue_throttle_khr)enums_test((L), DOMAIN_QUEUE_THROTTLE, (arg), (err))
#define checkqueuethrottle(L, arg) (cl_queue_throttle_khr)enums_check((L), DOMAIN_QUEUE_THROTTLE, (arg))
#define pushqueuethrottle(L, val) enums_push((L), DOMAIN_QUEUE_THROTTLE, (uint32_t)(val))
#define valuesqueuethrottle(L) enums_values((L), DOMAIN_QUEUE_THROTTLE)

#define testgltexturetarget(L, arg, err) (cl_int)enums_test((L), DOMAIN_GL_TEXTURE_TARGET, (arg), (err))
#define checkgltexturetarget(L, arg) (cl_int)enums_check((L), DOMAIN_GL_TEXTURE_TARGET, (arg))
#define pushgltexturetarget(L, val) enums_push((L), DOMAIN_GL_TEXTURE_TARGET, (uint32_t)(val))
#define valuesgltexturetarget(L) enums_values((L), DOMAIN_GL_TEXTURE_TARGET)

#define testglobjecttype(L, arg, err) (cl_gl_object_type)enums_test((L), DOMAIN_GL_OBJECT_TYPE, (arg), (err))
#define checkglobjecttype(L, arg) (cl_gl_object_type)enums_check((L), DOMAIN_GL_OBJECT_TYPE, (arg))
#define pushglobjecttype(L, val) enums_push((L), DOMAIN_GL_OBJECT_TYPE, (uint32_t)(val))
#define valuesglobjecttype(L) enums_values((L), DOMAIN_GL_OBJECT_TYPE)

#if 0 /* scaffolding 6yy */
#define testxxx(L, arg, err) (cl_xxx)enums_test((L), DOMAIN_XXX, (arg), (err))
#define checkxxx(L, arg) (cl_xxx)enums_check((L), DOMAIN_XXX, (arg))
#define pushxxx(L, val) enums_push((L), DOMAIN_XXX, (uint32_t)(val))
#define valuesxxx(L) enums_values((L), DOMAIN_XXX)
    CASE(xxx);

#endif

#endif /* enumsDEFINED */


