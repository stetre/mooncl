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

#ifndef getprocDEFINED
#define getprocDEFINED

#include "pfn.h"

#define CheckPfn(L, pfn, major, minor) do {                 \
    if(cl.pfn==NULL)                                        \
        return luaL_error((L),                              \
                "cl"#pfn" address not found (requires OpenCL version >= %d.%d)", \
                (major), (minor));                          \
} while(0)

#define CheckPfn_2_0(L, pfn) CheckPfn(L, pfn, 2, 0)
#define CheckPfn_2_1(L, pfn) CheckPfn(L, pfn, 2, 1)
#define CheckPfn_2_2(L, pfn) CheckPfn(L, pfn, 2, 2)

#define CheckExtPfn(L, ud_, pfn) do {                       \
    if((ud_)->clext->pfn==NULL)                             \
        return luaL_error((L), "cl"#pfn" extension address not found");\
} while(0)


/* Dispatch table for core functions */
typedef struct {
    PFN_clGetPlatformIDs GetPlatformIDs;
    PFN_clGetPlatformInfo GetPlatformInfo;
    PFN_clGetDeviceIDs GetDeviceIDs;
    PFN_clGetDeviceInfo GetDeviceInfo;
    PFN_clCreateSubDevices CreateSubDevices;
    PFN_clRetainDevice RetainDevice;
    PFN_clReleaseDevice ReleaseDevice;
    PFN_clSetDefaultDeviceCommandQueue SetDefaultDeviceCommandQueue;
    PFN_clGetDeviceAndHostTimer GetDeviceAndHostTimer;
    PFN_clGetHostTimer GetHostTimer;
    PFN_clCreateContext CreateContext;
    PFN_clCreateContextFromType CreateContextFromType;
    PFN_clRetainContext RetainContext;
    PFN_clReleaseContext ReleaseContext;
    PFN_clGetContextInfo GetContextInfo;
    PFN_clCreateCommandQueueWithProperties CreateCommandQueueWithProperties;
    PFN_clRetainCommandQueue RetainCommandQueue;
    PFN_clReleaseCommandQueue ReleaseCommandQueue;
    PFN_clGetCommandQueueInfo GetCommandQueueInfo;
    PFN_clCreateBuffer CreateBuffer;
    PFN_clCreateSubBuffer CreateSubBuffer;
    PFN_clCreateImage CreateImage;
    PFN_clCreatePipe CreatePipe;
    PFN_clRetainMemObject RetainMemObject;
    PFN_clReleaseMemObject ReleaseMemObject;
    PFN_clGetSupportedImageFormats GetSupportedImageFormats;
    PFN_clGetMemObjectInfo GetMemObjectInfo;
    PFN_clGetImageInfo GetImageInfo;
    PFN_clGetPipeInfo GetPipeInfo;
    PFN_clSetMemObjectDestructorCallback SetMemObjectDestructorCallback;
    PFN_clSVMAlloc SVMAlloc;
    PFN_clSVMFree SVMFree;
    PFN_clCreateSamplerWithProperties CreateSamplerWithProperties;
    PFN_clRetainSampler RetainSampler;
    PFN_clReleaseSampler ReleaseSampler;
    PFN_clGetSamplerInfo GetSamplerInfo;
    PFN_clCreateProgramWithSource CreateProgramWithSource;
    PFN_clCreateProgramWithBinary CreateProgramWithBinary;
    PFN_clCreateProgramWithBuiltInKernels CreateProgramWithBuiltInKernels;
    PFN_clCreateProgramWithIL CreateProgramWithIL;
    PFN_clRetainProgram RetainProgram;
    PFN_clReleaseProgram ReleaseProgram;
    PFN_clBuildProgram BuildProgram;
    PFN_clCompileProgram CompileProgram;
    PFN_clLinkProgram LinkProgram;
    PFN_clSetProgramReleaseCallback SetProgramReleaseCallback;
    PFN_clSetProgramSpecializationConstant SetProgramSpecializationConstant;
    PFN_clUnloadPlatformCompiler UnloadPlatformCompiler;
    PFN_clGetProgramInfo GetProgramInfo;
    PFN_clGetProgramBuildInfo GetProgramBuildInfo;
    PFN_clCreateKernel CreateKernel;
    PFN_clCreateKernelsInProgram CreateKernelsInProgram;
    PFN_clCloneKernel CloneKernel;
    PFN_clRetainKernel RetainKernel;
    PFN_clReleaseKernel ReleaseKernel;
    PFN_clSetKernelArg SetKernelArg;
    PFN_clSetKernelArgSVMPointer SetKernelArgSVMPointer;
    PFN_clSetKernelExecInfo SetKernelExecInfo;
    PFN_clGetKernelInfo GetKernelInfo;
    PFN_clGetKernelArgInfo GetKernelArgInfo;
    PFN_clGetKernelWorkGroupInfo GetKernelWorkGroupInfo;
    PFN_clGetKernelSubGroupInfo GetKernelSubGroupInfo;
    PFN_clWaitForEvents WaitForEvents;
    PFN_clGetEventInfo GetEventInfo;
    PFN_clCreateUserEvent CreateUserEvent;
    PFN_clRetainEvent RetainEvent;
    PFN_clReleaseEvent ReleaseEvent;
    PFN_clSetUserEventStatus SetUserEventStatus;
    PFN_clSetEventCallback SetEventCallback;
    PFN_clGetEventProfilingInfo GetEventProfilingInfo;
    PFN_clFlush Flush;
    PFN_clFinish Finish;
    PFN_clEnqueueReadBuffer EnqueueReadBuffer;
    PFN_clEnqueueReadBufferRect EnqueueReadBufferRect;
    PFN_clEnqueueWriteBuffer EnqueueWriteBuffer;
    PFN_clEnqueueWriteBufferRect EnqueueWriteBufferRect;
    PFN_clEnqueueFillBuffer EnqueueFillBuffer;
    PFN_clEnqueueCopyBuffer EnqueueCopyBuffer;
    PFN_clEnqueueCopyBufferRect EnqueueCopyBufferRect;
    PFN_clEnqueueReadImage EnqueueReadImage;
    PFN_clEnqueueWriteImage EnqueueWriteImage;
    PFN_clEnqueueFillImage EnqueueFillImage;
    PFN_clEnqueueCopyImage EnqueueCopyImage;
    PFN_clEnqueueCopyImageToBuffer EnqueueCopyImageToBuffer;
    PFN_clEnqueueCopyBufferToImage EnqueueCopyBufferToImage;
    PFN_clEnqueueMapBuffer EnqueueMapBuffer;
    PFN_clEnqueueMapImage EnqueueMapImage;
    PFN_clEnqueueUnmapMemObject EnqueueUnmapMemObject;
    PFN_clEnqueueMigrateMemObjects EnqueueMigrateMemObjects;
    PFN_clEnqueueNDRangeKernel EnqueueNDRangeKernel;
    PFN_clEnqueueNativeKernel EnqueueNativeKernel;
    PFN_clEnqueueMarkerWithWaitList EnqueueMarkerWithWaitList;
    PFN_clEnqueueBarrierWithWaitList EnqueueBarrierWithWaitList;
    PFN_clEnqueueSVMFree EnqueueSVMFree;
    PFN_clEnqueueSVMMemcpy EnqueueSVMMemcpy;
    PFN_clEnqueueSVMMemFill EnqueueSVMMemFill;
    PFN_clEnqueueSVMMap EnqueueSVMMap;
    PFN_clEnqueueSVMUnmap EnqueueSVMUnmap;
    PFN_clEnqueueSVMMigrateMem EnqueueSVMMigrateMem;
    PFN_clGetExtensionFunctionAddressForPlatform GetExtensionFunctionAddressForPlatform;
#if 1 //VERSION_1_2_DEPRECATED
    PFN_clCreateCommandQueue CreateCommandQueue;
    PFN_clCreateSampler CreateSampler;
    PFN_clEnqueueTask EnqueueTask;
#endif
#if 0 //DEPRECATED
    PFN_clCreateImage2D CreateImage2D;
    PFN_clCreateImage3D CreateImage3D;
    PFN_clEnqueueMarker EnqueueMarker;
    PFN_clEnqueueWaitForEvents EnqueueWaitForEvents;
    PFN_clEnqueueBarrier EnqueueBarrier;
    PFN_clUnloadCompiler UnloadCompiler;
    PFN_clGetExtensionFunctionAddress GetExtensionFunctionAddress;
#endif
} mooncl_dt_t;

#define cl mooncl_cl
extern mooncl_dt_t cl;

/* Dispatch table for extension functions */
typedef struct {
    clIcdGetPlatformIDsKHR_fn IcdGetPlatformIDsKHR;
    clTerminateContextKHR_fn TerminateContextKHR;
    clCreateEventFromGLsyncKHR_fn CreateEventFromGLsyncKHR;
    clCreateFromGLBuffer_fn CreateFromGLBuffer;
    clCreateFromGLTexture_fn CreateFromGLTexture;
    clCreateFromGLTexture2D_fn CreateFromGLTexture2D;
    clCreateFromGLTexture3D_fn CreateFromGLTexture3D;
    clCreateFromGLRenderbuffer_fn CreateFromGLRenderbuffer;
    clGetGLObjectInfo_fn GetGLObjectInfo;
    clGetGLTextureInfo_fn GetGLTextureInfo;
    clEnqueueAcquireGLObjects_fn EnqueueAcquireGLObjects;
    clEnqueueReleaseGLObjects_fn EnqueueReleaseGLObjects;
    clGetGLContextInfoKHR_fn GetGLContextInfoKHR;
    //clGetKernelSubGroupInfoKHR_fn GetKernelSubGroupInfoKHR;
} mooncl_extdt_t;



#endif /* getprocDEFINED */
