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

/*------------------------------------------------------------------------------*
 | Misc utilities                                                               |
 *------------------------------------------------------------------------------*/

int noprintf(const char *fmt, ...) 
    { (void)fmt; return 0; }

int notavailable(lua_State *L, ...) 
    { 
    return luaL_error(L, "function not available in this CL version");
    }

cl_bool is_little_endian(void)
    {
    uint32_t x = 0x12345678;
    return (((uint8_t*)&x)[0] == 0x78);
    }

/*------------------------------------------------------------------------------*
 | Malloc                                                                       |
 *------------------------------------------------------------------------------*/

/* We do not use malloc(), free() etc directly. Instead, we inherit the memory 
 * allocator from the main Lua state instead (see lua_getallocf in the Lua manual)
 * and use that.
 *
 * By doing so, we can use an alternative malloc() implementation without recompiling
 * this library (we have needs to recompile lua only, or execute it with LD_PRELOAD
 * set to the path to the malloc library we want to use).
 */
static lua_Alloc Alloc = NULL;
static void* AllocUd = NULL;

static void malloc_init(lua_State *L)
    {
    if(Alloc) unexpected(L);
    Alloc = lua_getallocf(L, &AllocUd);
    }

static void* Malloc_(size_t size)
    { return Alloc ? Alloc(AllocUd, NULL, 0, size) : NULL; }

static void Free_(void *ptr)
    { if(Alloc) Alloc(AllocUd, ptr, 0, 0); }

void *Malloc(lua_State *L, size_t size)
    {
    void *ptr = Malloc_(size);
    if(ptr==NULL)
        { luaL_error(L, errstring(ERR_MEMORY)); return NULL; }
    memset(ptr, 0, size);
    //DBG("Malloc %p\n", ptr);
    return ptr;
    }

void *MallocNoErr(lua_State *L, size_t size) /* do not raise errors (check the retval) */
    {
    void *ptr = Malloc_(size);
    (void)L;
    if(ptr==NULL)
        return NULL;
    memset(ptr, 0, size);
    //DBG("MallocNoErr %p\n", ptr);
    return ptr;
    }

char *Strdup(lua_State *L, const char *s)
    {
    size_t len = strnlen(s, 256);
    char *ptr = (char*)Malloc(L, len + 1);
    if(len>0)
        memcpy(ptr, s, len);
    ptr[len]='\0';
    return ptr;
    }


void Free(lua_State *L, void *ptr)
    {
    (void)L;
    //DBG("Free %p\n", ptr);
    if(ptr) Free_(ptr);
    }

/*------------------------------------------------------------------------------*
 | Time utilities                                                               |
 *------------------------------------------------------------------------------*/

#if defined(LINUX)

#if 0
static double tstosec(const struct timespec *ts)
    {
    return ts->tv_sec*1.0+ts->tv_nsec*1.0e-9;
    }
#endif

static void sectots(struct timespec *ts, double seconds)
    {
    ts->tv_sec=(time_t)seconds;
    ts->tv_nsec=(long)((seconds-((double)ts->tv_sec))*1.0e9);
    }

double now(void)
    {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0)
        { printf("clock_gettime error\n"); return -1; }
    return ts.tv_sec + ts.tv_nsec*1.0e-9;
#else
    struct timeval tv;
    if(gettimeofday(&tv, NULL) != 0)
        { printf("gettimeofday error\n"); return -1; }
    return tv.tv_sec + tv.tv_usec*1.0e-6;
#endif
    }

void sleeep(double seconds)
    {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts, ts1;
    struct timespec *req, *rem, *tmp;
    sectots(&ts, seconds);
    req = &ts;
    rem = &ts1;
    while(1)
        {
        if(nanosleep(req, rem) == 0)
            return;
        tmp = req;
        req = rem;
        rem = tmp;
        }
#else
    usleep((useconds_t)(seconds*1.0e6));
#endif
    }

#define time_init(L) do { (void)L; /* do nothing */ } while(0)

#elif defined(MINGW)

#include <windows.h>

static LARGE_INTEGER Frequency;
double now(void)
    {
    LARGE_INTEGER ts;
    QueryPerformanceCounter(&ts);
    return ((double)(ts.QuadPart))/Frequency.QuadPart;
    }

void sleeep(double seconds)
    {
    DWORD msec = (DWORD)seconds * 1000;
    //if(msec < 0) return;  DWORD seems to be unsigned
    Sleep(msec);
    }

static void time_init(lua_State *L)
    {
    (void)L;
    QueryPerformanceFrequency(&Frequency);
    }

#endif


/*------------------------------------------------------------------------------*
 | Light userdata                                                               |
 *------------------------------------------------------------------------------*/

void *checklightuserdata(lua_State *L, int arg)
    {
    if(lua_type(L, arg) != LUA_TLIGHTUSERDATA)
        { luaL_argerror(L, arg, "expected lightuserdata"); return NULL; }
    return lua_touserdata(L, arg);
    }
    
void *optlightuserdata(lua_State *L, int arg)
    {
    if(lua_isnoneornil(L, arg))
        return NULL;
    return checklightuserdata(L, arg);
    }

void *checklightuserdataorzero(lua_State *L, int arg)
    {
    int val, isnum;
    val = lua_tointegerx(L, arg, &isnum);
    if(!isnum)
        return checklightuserdata(L, arg);
    if(val != 0)
        luaL_argerror(L, arg, "expected lightuserdata or 0");
    return NULL;
    }


/*------------------------------------------------------------------------------*
 | Custom luaL_checkxxx() style functions                                       |
 *------------------------------------------------------------------------------*/

int checkboolean(lua_State *L, int arg)
    {
    if(!lua_isboolean(L, arg))
        return (int)luaL_argerror(L, arg, "boolean expected");
    return lua_toboolean(L, arg); // ? CL_TRUE : CL_FALSE;
    }


int testboolean(lua_State *L, int arg, int *err)
    {
    if(!lua_isboolean(L, arg))
        { *err = ERR_TYPE; return 0; }
    *err = 0;
    return lua_toboolean(L, arg); // ? CL_TRUE : CL_FALSE;
    }


int optboolean(lua_State *L, int arg, int d)
    {
    if(!lua_isboolean(L, arg))
        return d;
    return lua_toboolean(L, arg);
    }

#if 0
/* 1-based index to 0-based ------------------------------------------*/

int testindex(lua_State *L, int arg, int *err)
    {
    int val;
    if(!lua_isinteger(L, arg))
        { *err = ERR_TYPE; return 0; }
    val = lua_tointeger(L, arg);
    if(val < 1)
        { *err = ERR_GENERIC; return 0; }
    *err = 0;
    return val - 1;
    }

int checkindex(lua_State *L, int arg)
    {
    int val = luaL_checkinteger(L, arg);
    if(val < 1)
        return luaL_argerror(L, arg, "positive integer expected");
    return val - 1;
    }

int optindex(lua_State *L, int arg, int optval /* 0-based */)
    {
    int val = luaL_optinteger(L, arg, optval + 1);
    if(val < 1)
        return luaL_argerror(L, arg, "positive integer expected");
    return val - 1;
    }

void pushindex(lua_State *L, int val)
    { lua_pushinteger((L), (val) + 1); }
#endif

/*------------------------------------------------------------------------------*
 | Internal error codes                                                         |
 *------------------------------------------------------------------------------*/

const char* errstring(int err)
    {
    switch(err)
        {
        case 0: return "success";
        case ERR_GENERIC: return "generic error";
        case ERR_TABLE: return "not a table";
        case ERR_EMPTY: return "empty list";
        case ERR_TYPE: return "invalid type";
        case ERR_VALUE: return "invalid value";
        case ERR_NOTPRESENT: return "missing";
        case ERR_MEMORY: return "out of memory";
        case ERR_LENGTH: return "invalid length";
        case ERR_POOL: return "elements are not from the same pool";
        case ERR_BOUNDARIES: return "invalid boundaries";
        case ERR_UNKNOWN: return "unknown field name";
        default:
            return "???";
        }
    return NULL; /* unreachable */
    }

/*------------------------------------------------------------------------------*
 | OpenCL error codes                                                           |
 *------------------------------------------------------------------------------*/

int pusherrcode(lua_State *L, cl_int ec)
    {
    switch(ec)
        {
#define CASE(c, s) case c: lua_pushstring(L, s); return 1
        CASE(CL_SUCCESS, "success");
        CASE(CL_DEVICE_NOT_FOUND, "device not found");
        CASE(CL_DEVICE_NOT_AVAILABLE, "device not available");
        CASE(CL_COMPILER_NOT_AVAILABLE, "compiler not available");
        CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE, "mem object allocation failure");
        CASE(CL_OUT_OF_RESOURCES, "out of resources");
        CASE(CL_OUT_OF_HOST_MEMORY, "out of host memory");
        CASE(CL_PROFILING_INFO_NOT_AVAILABLE, "profiling info not available");
        CASE(CL_MEM_COPY_OVERLAP, "mem copy overlap");
        CASE(CL_IMAGE_FORMAT_MISMATCH, "image format mismatch");
        CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED, "image format not supported");
        CASE(CL_BUILD_PROGRAM_FAILURE, "build program failure");
        CASE(CL_MAP_FAILURE, "map failure");
        CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET, "misaligned sub buffer offset");
        CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, "exec status error for events in wait list");
        CASE(CL_COMPILE_PROGRAM_FAILURE, "compile program failure");
        CASE(CL_LINKER_NOT_AVAILABLE, "linker not available");
        CASE(CL_LINK_PROGRAM_FAILURE, "link program failure");
        CASE(CL_DEVICE_PARTITION_FAILED, "device partition failed");
        CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE, "kernel arg info not available");
        CASE(CL_INVALID_VALUE, "invalid value");
        CASE(CL_INVALID_DEVICE_TYPE, "invalid device type");
        CASE(CL_INVALID_PLATFORM, "invalid platform");
        CASE(CL_INVALID_DEVICE, "invalid device");
        CASE(CL_INVALID_CONTEXT, "invalid context");
        CASE(CL_INVALID_QUEUE_PROPERTIES, "invalid queue properties");
        CASE(CL_INVALID_COMMAND_QUEUE, "invalid command queue");
        CASE(CL_INVALID_HOST_PTR, "invalid host ptr");
        CASE(CL_INVALID_MEM_OBJECT, "invalid mem object");
        CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR, "invalid image format descriptor");
        CASE(CL_INVALID_IMAGE_SIZE, "invalid image size");
        CASE(CL_INVALID_SAMPLER, "invalid sampler");
        CASE(CL_INVALID_BINARY, "invalid binary");
        CASE(CL_INVALID_BUILD_OPTIONS, "invalid build options");
        CASE(CL_INVALID_PROGRAM, "invalid program");
        CASE(CL_INVALID_PROGRAM_EXECUTABLE, "invalid program executable");
        CASE(CL_INVALID_KERNEL_NAME, "invalid kernel name");
        CASE(CL_INVALID_KERNEL_DEFINITION, "invalid kernel definition");
        CASE(CL_INVALID_KERNEL, "invalid kernel");
        CASE(CL_INVALID_ARG_INDEX, "invalid arg index");
        CASE(CL_INVALID_ARG_VALUE, "invalid arg value");
        CASE(CL_INVALID_ARG_SIZE, "invalid arg size");
        CASE(CL_INVALID_KERNEL_ARGS, "invalid kernel args");
        CASE(CL_INVALID_WORK_DIMENSION, "invalid work dimension");
        CASE(CL_INVALID_WORK_GROUP_SIZE, "invalid work group size");
        CASE(CL_INVALID_WORK_ITEM_SIZE, "invalid work item size");
        CASE(CL_INVALID_GLOBAL_OFFSET, "invalid global offset");
        CASE(CL_INVALID_EVENT_WAIT_LIST, "invalid event wait list");
        CASE(CL_INVALID_EVENT, "invalid event");
        CASE(CL_INVALID_OPERATION, "invalid operation");
        CASE(CL_INVALID_GL_OBJECT, "invalid gl object");
        CASE(CL_INVALID_BUFFER_SIZE, "invalid buffer size");
        CASE(CL_INVALID_MIP_LEVEL, "invalid mip level");
        CASE(CL_INVALID_GLOBAL_WORK_SIZE, "invalid global work size");
        CASE(CL_INVALID_PROPERTY, "invalid property");
        CASE(CL_INVALID_IMAGE_DESCRIPTOR, "invalid image descriptor");
        CASE(CL_INVALID_COMPILER_OPTIONS, "invalid compiler options");
        CASE(CL_INVALID_LINKER_OPTIONS, "invalid linker options");
        CASE(CL_INVALID_DEVICE_PARTITION_COUNT, "invalid device partition count");
        CASE(CL_INVALID_PIPE_SIZE, "invalid pipe size");
        CASE(CL_INVALID_DEVICE_QUEUE, "invalid device queue");
        /* Additional error codes ------------------------------------*/
#ifdef CL_PLATFORM_NOT_FOUND_KHR /* cl_ext.h */
        CASE(CL_PLATFORM_NOT_FOUND_KHR, "platform not found");
#endif
#ifdef CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR /* cl_gl.h */
        CASE(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR, "invalid gl sharegroup reference");
#endif

#ifdef CL_INVALID_EGL_OBJECT_KHR /* cl_egl.h */
        CASE(CL_INVALID_EGL_OBJECT_KHR, "invalid egl object");
        CASE(CL_EGL_RESOURCE_NOT_ACQUIRED_KHR, "egl resource not acquired");
#endif
#ifdef CL_INVALID_D3D10_DEVICE_KHR /* cl_d3d10.h */
        CASE(CL_INVALID_D3D10_DEVICE_KHR, "invalid d3d10 device");
        CASE(CL_INVALID_D3D10_RESOURCE_KHR, "invalid d3d10 resource");
        CASE(CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR, "d3d10 resource already acquired");
        CASE(CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR, "d3d10 resource not acquired");
#endif
#ifdef CL_INVALID_D3D11_DEVICE_KHR /* cl_d3d11.h */
        CASE(CL_INVALID_D3D11_DEVICE_KHR, "invalid d3d11 device");
        CASE(CL_INVALID_D3D11_RESOURCE_KHR, "invalid d3d11 resource");
        CASE(CL_D3D11_RESOURCE_ALREADY_ACQUIRED_KHR, "d3d11 resource already acquired");
        CASE(CL_D3D11_RESOURCE_NOT_ACQUIRED_KHR, "d3d11 resource not acquired");
#endif
#ifdef CL_INVALID_DX9_MEDIA_ADAPTER_KHR /* cl_dx9_media_sharing.h */
        CASE(CL_INVALID_DX9_MEDIA_ADAPTER_KHR, "invalid dx9 media adapter");
        CASE(CL_INVALID_DX9_MEDIA_SURFACE_KHR, "invalid dx9 media surface");
        CASE(CL_DX9_MEDIA_SURFACE_ALREADY_ACQUIRED_KHR, "dx9 media surface already acquired");
        CASE(CL_DX9_MEDIA_SURFACE_NOT_ACQUIRED_KHR, "dx9 media surface not acquired");
#endif
#ifdef CL_INVALID_SPEC_ID
        CASE(CL_INVALID_SPEC_ID, "invalid spec id");
#endif
#ifdef CL_MAX_SIZE_RESTRICTION_EXCEEDED
        CASE(CL_MAX_SIZE_RESTRICTION_EXCEEDED, "max size restriction exceeded");
#endif
        CASE(-9999, "illegal read or write to a buffer"); /* nvidia wtf */
#if 0
        CASE(, "");
#endif
#undef CASE
        default:
            lua_pushfstring(L, "unknown OpenCL error code %d", ec); 
            return 1;
            //return unexpected(L);
        }
    return 0;
    }

/*------------------------------------------------------------------------------*
 | Inits                                                                        |
 *------------------------------------------------------------------------------*/

void mooncl_utils_init(lua_State *L)
    {
    malloc_init(L);
    time_init(L);
    }

