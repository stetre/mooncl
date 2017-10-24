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

mooncl_dt_t cl;

#define FP(f) *(void**)(&(f))
/* Cast to silent compiler warnings without giving up the -Wpedantic flag.
 *("ISO C forbids conversion of function pointer to object pointer type")
 */

#if defined(LINUX)
#include <dlfcn.h>
#define LIBNAME "libOpenCL.so"
static void *Handle = NULL;

#elif defined(MINGW)
#include <windows.h>
#define LIBNAME "OpenCL.dll"
#define LLIBNAME L"OpenCL.dll"
static HMODULE Handle = NULL;
#ifdef CreateWindow /* damn MS... */
#undef CreateWindow
#endif

#else
#error "Cannot determine platform"
#endif


static int Init(lua_State *L)
    {
#if defined(LINUX)
    char *err;
    Handle = dlopen(LIBNAME, RTLD_LAZY | RTLD_LOCAL);
    if(!Handle)
        {
        err = dlerror();
        return luaL_error(L, err != NULL ? err : "cannot load "LIBNAME);
        }
#define GET(fn) do {                                            \
    FP(cl.fn) = dlsym(Handle, "cl"#fn);                         \
    if(!cl.fn) return luaL_error(L, "cannot find cl"#fn);       \
} while(0)
#define OPT(fn) do {    /* optional */                          \
    FP(cl.fn) = dlsym(Handle, "cl"#fn);                         \
} while(0)

#elif defined(MINGW)
    Handle = LoadLibraryW(LLIBNAME);
    if(!Handle)
        return luaL_error(L, "cannot load "LIBNAME);
#define GET(fn) do {                                          \
    cl.fn = (PFN_cl##fn)GetProcAddress(Handle, "cl"#fn);      \
    if(!cl.fn) return luaL_error(L, "cannot find cl"#fn);     \
} while(0)
#define OPT(fn) do {    /* optional */                        \
    cl.fn = (PFN_cl##fn)GetProcAddress(Handle, "cl"#fn);      \
} while(0)
#endif

    /* Fill the global dispatch table */
    /* If MoonCL loads successfully, these function pointers retrieved with 
     * GET() are guaranteed to be valid so they need not be checked before 
     * using them, while those retrieved with OPT() may be NULL so they must
     * be checked before using them.
     */
    GET(GetPlatformIDs); //VERSION_1_0
    GET(GetPlatformInfo); //VERSION_1_0
    GET(GetDeviceIDs); //VERSION_1_0
    GET(GetDeviceInfo); //VERSION_1_0
    GET(CreateContext); //VERSION_1_0
    GET(CreateContextFromType); //VERSION_1_0
    GET(RetainContext); //VERSION_1_0
    GET(ReleaseContext); //VERSION_1_0
    GET(GetContextInfo); //VERSION_1_0
    GET(RetainCommandQueue); //VERSION_1_0
    GET(ReleaseCommandQueue); //VERSION_1_0
    GET(GetCommandQueueInfo); //VERSION_1_0
    GET(CreateBuffer); //VERSION_1_0
    GET(RetainMemObject); //VERSION_1_0
    GET(ReleaseMemObject); //VERSION_1_0
    GET(GetSupportedImageFormats); //VERSION_1_0
    GET(GetMemObjectInfo); //VERSION_1_0
    GET(GetImageInfo); //VERSION_1_0
    GET(RetainSampler); //VERSION_1_0
    GET(ReleaseSampler); //VERSION_1_0
    GET(GetSamplerInfo); //VERSION_1_0
    GET(CreateProgramWithSource); //VERSION_1_0
    GET(CreateProgramWithBinary); //VERSION_1_0
    GET(RetainProgram); //VERSION_1_0
    GET(ReleaseProgram); //VERSION_1_0
    GET(BuildProgram); //VERSION_1_0
    GET(GetProgramInfo); //VERSION_1_0
    GET(GetProgramBuildInfo); //VERSION_1_0
    GET(CreateKernel); //VERSION_1_0
    GET(CreateKernelsInProgram); //VERSION_1_0
    GET(RetainKernel); //VERSION_1_0
    GET(ReleaseKernel); //VERSION_1_0
    GET(SetKernelArg); //VERSION_1_0
    GET(GetKernelInfo); //VERSION_1_0
    GET(GetKernelWorkGroupInfo); //VERSION_1_0
    GET(WaitForEvents); //VERSION_1_0
    GET(GetEventInfo); //VERSION_1_0
    GET(RetainEvent); //VERSION_1_0
    GET(ReleaseEvent); //VERSION_1_0
    GET(GetEventProfilingInfo); //VERSION_1_0
    GET(Flush); //VERSION_1_0
    GET(Finish); //VERSION_1_0
    GET(EnqueueReadBuffer); //VERSION_1_0
    GET(EnqueueWriteBuffer); //VERSION_1_0
    GET(EnqueueCopyBuffer); //VERSION_1_0
    GET(EnqueueReadImage); //VERSION_1_0
    GET(EnqueueWriteImage); //VERSION_1_0
    GET(EnqueueCopyImage); //VERSION_1_0
    GET(EnqueueCopyImageToBuffer); //VERSION_1_0
    GET(EnqueueCopyBufferToImage); //VERSION_1_0
    GET(EnqueueMapBuffer); //VERSION_1_0
    GET(EnqueueMapImage); //VERSION_1_0
    GET(EnqueueUnmapMemObject); //VERSION_1_0
    GET(EnqueueNDRangeKernel); //VERSION_1_0
    GET(EnqueueNativeKernel); //VERSION_1_0

    GET(EnqueueReadBufferRect); //VERSION_1_1
    GET(SetMemObjectDestructorCallback); //VERSION_1_1
    GET(CreateSubBuffer); //VERSION_1_1
    GET(CreateUserEvent); //VERSION_1_1;
    GET(SetUserEventStatus); //VERSION_1_1
    GET(SetEventCallback); //VERSION_1_1
    GET(EnqueueWriteBufferRect); //VERSION_1_1
    GET(EnqueueCopyBufferRect); //VERSION_1_1

    GET(CreateImage); //VERSION_1_2
    GET(CreateSubDevices); //VERSION_1_2
    GET(RetainDevice); //VERSION_1_2
    GET(ReleaseDevice); //VERSION_1_2
    GET(CreateProgramWithBuiltInKernels); //VERSION_1_2
    GET(CompileProgram); //VERSION_1_2
    GET(LinkProgram); //VERSION_1_2
    GET(UnloadPlatformCompiler); //VERSION_1_2
    GET(GetKernelArgInfo); //VERSION_1_2
    GET(EnqueueFillBuffer); //VERSION_1_2
    GET(EnqueueFillImage); //VERSION_1_2
    GET(EnqueueMigrateMemObjects); //VERSION_1_2
    GET(EnqueueMarkerWithWaitList); //VERSION_1_2
    GET(EnqueueBarrierWithWaitList); //VERSION_1_2
    GET(GetExtensionFunctionAddressForPlatform); //VERSION_1_2

    GET(CreateCommandQueue); //VERSION_1_2_DEPRECATED
    GET(CreateSampler); //VERSION_1_2_DEPRECATED
    GET(EnqueueTask); //VERSION_1_2_DEPRECATED

    OPT(CreateCommandQueueWithProperties); //VERSION_2_0
    OPT(CreatePipe); //VERSION_2_0
    OPT(GetPipeInfo); //VERSION_2_0
    OPT(SVMAlloc); //VERSION_2_0
    OPT(SVMFree); //VERSION_2_0
    OPT(CreateSamplerWithProperties); //VERSION_2_0
    OPT(SetKernelArgSVMPointer); //VERSION_2_0
    OPT(SetKernelExecInfo); //VERSION_2_0
    OPT(EnqueueSVMFree); //VERSION_2_0
    OPT(EnqueueSVMMemcpy); //VERSION_2_0
    OPT(EnqueueSVMMemFill); //VERSION_2_0
    OPT(EnqueueSVMMap); //VERSION_2_0
    OPT(EnqueueSVMUnmap); //VERSION_2_0

    OPT(SetDefaultDeviceCommandQueue); //VERSION_2_1
    OPT(GetDeviceAndHostTimer); //VERSION_2_1
    OPT(GetHostTimer); //VERSION_2_1
    OPT(CreateProgramWithIL); //VERSION_2_1
    OPT(CloneKernel); //VERSION_2_1
    OPT(GetKernelSubGroupInfo); //VERSION_2_1
    OPT(EnqueueSVMMigrateMem); //VERSION_2_1

    OPT(SetProgramReleaseCallback); //VERSION_2_2
    OPT(SetProgramSpecializationConstant); //VERSION_2_2

//  GET(CreateImage2D); //VERSION_1_1_DEPRECATED
//  GET(CreateImage3D); //VERSION_1_1_DEPRECATED
//  GET(EnqueueMarker); //VERSION_1_1_DEPRECATED
//  GET(EnqueueWaitForEvents); //VERSION_1_1_DEPRECATED
//  GET(EnqueueBarrier); //VERSION_1_1_DEPRECATED
//  GET(UnloadCompiler); //VERSION_1_1_DEPRECATED
//  GET(GetExtensionFunctionAddress); //VERSION_1_1_DEPRECATED

#undef GET
#undef OPT
    return 0;
    }

mooncl_extdt_t *mooncl_getproc_extensions(lua_State *L, cl_platform platform)
/* Creates a dispatch table for platform.
 * The dispatch table contains the function pointers for all extension functions.
 * If a function is provided by an extension that is not supported by platform,
 * then its function pointer is NULL (ie, check before use).
 */
    {
#define OPT(fn) /* all extensions are optional */ do {  \
    FP(clext->fn) = cl.GetExtensionFunctionAddressForPlatform(platform, "cl"#fn);   \
    /* printf("cl"#fn" %p\n", FP(clext->fn)); */\
} while(0)
    mooncl_extdt_t *clext = (mooncl_extdt_t*)Malloc(L, sizeof(mooncl_extdt_t));
    OPT(IcdGetPlatformIDsKHR);
    OPT(TerminateContextKHR);
    OPT(CreateEventFromGLsyncKHR);
    OPT(CreateFromGLBuffer);
    OPT(CreateFromGLTexture);
    OPT(CreateFromGLTexture2D);
    OPT(CreateFromGLTexture3D);
    OPT(CreateFromGLRenderbuffer);
    OPT(GetGLObjectInfo);
    OPT(GetGLTextureInfo);
    OPT(EnqueueAcquireGLObjects);
    OPT(EnqueueReleaseGLObjects);
    OPT(GetGLContextInfoKHR);
    //OPT(GetKernelSubGroupInfoKHR);
    return clext;
#undef OPT
    }

void mooncl_atexit_getproc(void)
    {
#if defined(LINUX)
    if(Handle) dlclose(Handle);
#elif defined(MINGW)
    if(Handle) FreeLibrary(Handle);
#endif
    }

/*---------------------------------------------------------------------------*/

int mooncl_open_getproc(lua_State *L)
    {
    Init(L);
    return 0;
    }
