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

#ifndef pfnDEFINED
#define pfnDEFINED

#include "mooncl.h"

typedef cl_int (*PFN_clGetPlatformIDs)(cl_uint, cl_platform_id*, cl_uint*);
typedef cl_int (*PFN_clGetPlatformInfo)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetDeviceIDs)(cl_platform_id,cl_device_type,cl_uint,cl_device_id*,cl_uint*);
typedef cl_int (*PFN_clGetDeviceInfo)(cl_device_id,cl_device_info,size_t,void*,size_t*);
typedef cl_int (*PFN_clCreateSubDevices)(cl_device_id,const cl_device_partition_property*,cl_uint,cl_device_id*,cl_uint*);
typedef cl_int (*PFN_clRetainDevice)(cl_device_id);
typedef cl_int (*PFN_clReleaseDevice)(cl_device_id);
typedef cl_int (*PFN_clSetDefaultDeviceCommandQueue)(cl_context,cl_device_id,cl_command_queue);
typedef cl_int (*PFN_clGetDeviceAndHostTimer)(cl_device_id,cl_ulong*,cl_ulong*);
typedef cl_int (*PFN_clGetHostTimer)(cl_device_id,cl_ulong*);
typedef cl_context (*PFN_clCreateContext)(const cl_context_properties*,cl_uint,const cl_device_id*,void (*)(const char*, const void*, size_t, void*),void*,cl_int*);
typedef cl_context (*PFN_clCreateContextFromType)(const cl_context_properties*,cl_device_type,void (*)(const char*, const void*, size_t, void*),void*,cl_int*);
typedef cl_int (*PFN_clRetainContext)(cl_context);
typedef cl_int (*PFN_clReleaseContext)(cl_context);
typedef cl_int (*PFN_clGetContextInfo)(cl_context, cl_context_info, size_t, void*, size_t*);
typedef cl_command_queue (*PFN_clCreateCommandQueueWithProperties)(cl_context,cl_device_id,const cl_queue_properties*,cl_int*);
typedef cl_int (*PFN_clRetainCommandQueue)(cl_command_queue);
typedef cl_int (*PFN_clReleaseCommandQueue)(cl_command_queue);
typedef cl_int (*PFN_clGetCommandQueueInfo)(cl_command_queue, cl_command_queue_info, size_t, void*, size_t*);
typedef cl_mem (*PFN_clCreateBuffer)(cl_context,cl_mem_flags,size_t,void*,cl_int*);
typedef cl_mem (*PFN_clCreateSubBuffer)(cl_mem, cl_mem_flags, cl_buffer_create_type, const void*, cl_int*);
typedef cl_mem (*PFN_clCreateImage)(cl_context, cl_mem_flags, const cl_image_format*, const cl_image_desc*, void*, cl_int*);
typedef cl_mem (*PFN_clCreatePipe)(cl_context, cl_mem_flags, cl_uint, cl_uint, const cl_pipe_properties*, cl_int*);
typedef cl_int (*PFN_clRetainMemObject)(cl_mem);
typedef cl_int (*PFN_clReleaseMemObject)(cl_mem);
typedef cl_int (*PFN_clGetSupportedImageFormats)(cl_context, cl_mem_flags, cl_mem_object_type, cl_uint, cl_image_format*, cl_uint*);
typedef cl_int (*PFN_clGetMemObjectInfo)(cl_mem, cl_mem_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetImageInfo)(cl_mem, cl_image_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetPipeInfo)(cl_mem,cl_pipe_info,size_t,void*,size_t*);
typedef cl_int (*PFN_clSetMemObjectDestructorCallback)(cl_mem, void (*)( cl_mem, void*), void*);
typedef void* (*PFN_clSVMAlloc)(cl_context, cl_svm_mem_flags, size_t, cl_uint);
typedef void (*PFN_clSVMFree)(cl_context,void*);
typedef cl_sampler (*PFN_clCreateSamplerWithProperties)(cl_context, const cl_sampler_properties*,cl_int*);
typedef cl_int (*PFN_clRetainSampler)(cl_sampler);
typedef cl_int (*PFN_clReleaseSampler)(cl_sampler);
typedef cl_int (*PFN_clGetSamplerInfo)(cl_sampler, cl_sampler_info, size_t, void*, size_t*);
typedef cl_program (*PFN_clCreateProgramWithSource)(cl_context, cl_uint, const char**, const size_t*, cl_int*);
typedef cl_program (*PFN_clCreateProgramWithBinary)(cl_context, cl_uint, const cl_device_id*, const size_t*, const unsigned char**, cl_int*, cl_int*);
typedef cl_program (*PFN_clCreateProgramWithBuiltInKernels)(cl_context, cl_uint, const cl_device_id*, const char*, cl_int*);
typedef cl_program (*PFN_clCreateProgramWithIL)(cl_context, const void*, size_t, cl_int*);
typedef cl_int (*PFN_clRetainProgram)(cl_program);
typedef cl_int (*PFN_clReleaseProgram)(cl_program);
typedef cl_int (*PFN_clBuildProgram)(cl_program, cl_uint, const cl_device_id*, const char*, void (*)(cl_program, void*), void*);
typedef cl_int (*PFN_clCompileProgram)(cl_program, cl_uint, const cl_device_id*, const char*, cl_uint, const cl_program*, const char**, void (*)(cl_program, void*), void*);
typedef cl_program (*PFN_clLinkProgram)(cl_context, cl_uint, const cl_device_id*, const char*, cl_uint, const cl_program*, void (*)(cl_program, void*), void*, cl_int*);
typedef cl_int (*PFN_clSetProgramReleaseCallback)(cl_program, void (*)(cl_program, void*), void*);
typedef cl_int (*PFN_clSetProgramSpecializationConstant)(cl_program, cl_uint, size_t, const void*);
typedef cl_int (*PFN_clUnloadPlatformCompiler)(cl_platform_id);
typedef cl_int (*PFN_clGetProgramInfo)(cl_program, cl_program_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetProgramBuildInfo)(cl_program, cl_device_id, cl_program_build_info, size_t, void*, size_t*);
typedef cl_kernel (*PFN_clCreateKernel)(cl_program, const char*, cl_int*);
typedef cl_int (*PFN_clCreateKernelsInProgram)(cl_program, cl_uint, cl_kernel*, cl_uint*);
typedef cl_kernel (*PFN_clCloneKernel)(cl_kernel, cl_int*);
typedef cl_int (*PFN_clRetainKernel)(cl_kernel);
typedef cl_int (*PFN_clReleaseKernel)(cl_kernel);
typedef cl_int (*PFN_clSetKernelArg)(cl_kernel, cl_uint, size_t, const void*);
typedef cl_int (*PFN_clSetKernelArgSVMPointer)(cl_kernel, cl_uint, const void*);
typedef cl_int (*PFN_clSetKernelExecInfo)(cl_kernel, cl_kernel_exec_info, size_t, const void*);
typedef cl_int (*PFN_clGetKernelInfo)(cl_kernel, cl_kernel_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetKernelArgInfo)(cl_kernel, cl_uint, cl_kernel_arg_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetKernelWorkGroupInfo)(cl_kernel, cl_device_id, cl_kernel_work_group_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clGetKernelSubGroupInfo)(cl_kernel, cl_device_id, cl_kernel_sub_group_info, size_t, const void*, size_t, void*, size_t*);
typedef cl_int (*PFN_clWaitForEvents)(cl_uint, const cl_event*);
typedef cl_int (*PFN_clGetEventInfo)(cl_event, cl_event_info, size_t, void*, size_t*);
typedef cl_event (*PFN_clCreateUserEvent)(cl_context, cl_int*);
typedef cl_int (*PFN_clRetainEvent)(cl_event);
typedef cl_int (*PFN_clReleaseEvent)(cl_event);
typedef cl_int (*PFN_clSetUserEventStatus)(cl_event, cl_int);
typedef cl_int (*PFN_clSetEventCallback)( cl_event, cl_int, void (*)(cl_event, cl_int, void*), void*);
typedef cl_int (*PFN_clGetEventProfilingInfo)(cl_event, cl_profiling_info, size_t, void*, size_t*);
typedef cl_int (*PFN_clFlush)(cl_command_queue);
typedef cl_int (*PFN_clFinish)(cl_command_queue);
typedef cl_int (*PFN_clEnqueueReadBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueReadBufferRect)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueWriteBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueWriteBufferRect)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueFillBuffer)(cl_command_queue, cl_mem, const void*, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueCopyBuffer)(cl_command_queue, cl_mem, cl_mem, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueCopyBufferRect)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueReadImage)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueWriteImage)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueFillImage)(cl_command_queue, cl_mem, const void*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueCopyImage)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueCopyImageToBuffer)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueCopyBufferToImage)(cl_command_queue, cl_mem, cl_mem, size_t, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef void* (*PFN_clEnqueueMapBuffer)(cl_command_queue, cl_mem, cl_bool, cl_map_flags, size_t, size_t, cl_uint, const cl_event*, cl_event*, cl_int*);
typedef void* (*PFN_clEnqueueMapImage)(cl_command_queue, cl_mem, cl_bool, cl_map_flags, const size_t*, const size_t*, size_t*, size_t*, cl_uint, const cl_event*, cl_event*, cl_int*);
typedef cl_int (*PFN_clEnqueueUnmapMemObject)(cl_command_queue, cl_mem, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueMigrateMemObjects)(cl_command_queue, cl_uint, const cl_mem*, cl_mem_migration_flags, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueNDRangeKernel)(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueNativeKernel)(cl_command_queue, void (*)(void*), void*, size_t, cl_uint, const cl_mem*, const void**, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueMarkerWithWaitList)(cl_command_queue, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueBarrierWithWaitList)(cl_command_queue, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMFree)(cl_command_queue, cl_uint, void*[], void (*)(cl_command_queue, cl_uint, void*[], void*), void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMMemcpy)(cl_command_queue, cl_bool, void*, const void*, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMMemFill)(cl_command_queue, void*, const void*, size_t, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMMap)(cl_command_queue, cl_bool, cl_map_flags, void*, size_t, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMUnmap)(cl_command_queue, void*, cl_uint, const cl_event*, cl_event*);
typedef cl_int (*PFN_clEnqueueSVMMigrateMem)(cl_command_queue, cl_uint, const void**, const size_t*, cl_mem_migration_flags, cl_uint, const cl_event*, cl_event*);
typedef void*(*PFN_clGetExtensionFunctionAddressForPlatform)(cl_platform_id, const char*);
#if 0 //DEPRECATED
typedef cl_mem (*PFN_clCreateImage2D)(cl_context, cl_mem_flags, const cl_image_format*, size_t, size_t, size_t, void*, cl_int*);
typedef cl_mem (*PFN_clCreateImage3D)(cl_context, cl_mem_flags, const cl_image_format*, size_t, size_t, size_t, size_t, size_t, void*, cl_int*);
typedef cl_int (*PFN_clEnqueueMarker)(cl_command_queue, cl_event*);
typedef cl_int (*PFN_clEnqueueWaitForEvents)(cl_command_queue, cl_uint, const cl_event*);
typedef cl_int (*PFN_clEnqueueBarrier)(cl_command_queue);
typedef cl_int (*PFN_clUnloadCompiler)(void);
typedef void* (*PFN_clGetExtensionFunctionAddress)(const char*);
#endif

#if 1 //VERSION_1_2_DEPRECATED
typedef cl_command_queue (*PFN_clCreateCommandQueue)(cl_context, cl_device_id, cl_command_queue_properties, cl_int*);
typedef cl_sampler (*PFN_clCreateSampler)(cl_context, cl_bool, cl_addressing_mode, cl_filter_mode, cl_int*);
typedef cl_int (*PFN_clEnqueueTask)(cl_command_queue, cl_kernel, cl_uint, const cl_event*, cl_event*);
#endif

typedef cl_event (*clCreateEventFromGLsyncKHR_fn)(cl_context, cl_GLsync, cl_int*); //VERSION_1_1
typedef cl_mem (*clCreateFromGLBuffer_fn)(cl_context, cl_mem_flags, cl_GLuint, int*); //VERSION_1_0
typedef cl_mem (*clCreateFromGLTexture_fn)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int*); //VERSION_1_2
typedef cl_mem (*clCreateFromGLRenderbuffer_fn)(cl_context, cl_mem_flags, cl_GLuint, cl_int*); //VERSION_1_0
typedef cl_int (*clGetGLObjectInfo_fn)(cl_mem, cl_gl_object_type*, cl_GLuint*); //VERSION_1_0
typedef cl_int (*clGetGLTextureInfo_fn)(cl_mem, cl_gl_texture_info, size_t, void*, size_t*); //VERSION_1_0
typedef cl_int (*clEnqueueAcquireGLObjects_fn)(cl_command_queue, cl_uint, const cl_mem*, cl_uint, const cl_event*, cl_event*); //VERSION_1_0
typedef cl_int (*clEnqueueReleaseGLObjects_fn)(cl_command_queue, cl_uint, const cl_mem*, cl_uint, const cl_event*, cl_event*); //VERSION_1_0
//typedef cl_int (*clGetGLContextInfoKHR_fn)(const cl_context_properties*, cl_gl_context_info, size_t, void*, size_t*); //VERSION_1_0
//typedef cl_int (*clGetKernelSubGroupInfoKHR_fn)(cl_kernel, cl_device_id, cl_kernel_sub_group_info, size_t , const void*, size_t, void*, size_t*); // VERSION_2_0_DEPRECATED
 


#endif /* pfnDEFINED */
