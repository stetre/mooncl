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

#ifndef objectsDEFINED
#define objectsDEFINED

#include "tree.h"
#include "udata.h"

/* CL TYPES ADAPTATION (for internal use) ------------- */

/* get rid of _id suffixes: */
#define cl_platform cl_platform_id
#define cl_device cl_device_id

/* internal alias ... */
#define cl_queue cl_command_queue
#define cl_queue_info cl_command_queue_info

/* specialized cl_mem types: */
#define cl_image cl_mem
#define cl_buffer cl_mem
#define cl_pipe cl_mem

/* encapsulation type for svm: */
typedef struct {
    void *ptr;
    cl_svm_mem_flags flags;
    cl_uint alignment;
    size_t size;
} svm_t;
#define cl_svm svm_t*

/* host accessible memory: */
typedef struct {
    char *ptr; 
    size_t size;
} hostmem_t;
#define cl_hostmem hostmem_t*

/*------------------------------------------------------*/

/* Objects' metatable names */
#define PLATFORM_MT "mooncl_platform"
#define DEVICE_MT "mooncl_device"
#define CONTEXT_MT "mooncl_context"
#define COMMAND_QUEUE_MT "mooncl_queue"
#define BUFFER_MT "mooncl_buffer"
#define IMAGE_MT "mooncl_image"
#define PIPE_MT "mooncl_pipe"
#define PROGRAM_MT "mooncl_program"
#define KERNEL_MT "mooncl_kernel"
#define EVENT_MT "mooncl_event"
#define SAMPLER_MT "mooncl_sampler"
#define SVM_MT "mooncl_svm"
#define HOSTMEM_MT "mooncl_hostmem"

/* Userdata memory associated with objects */
#define ud_t mooncl_ud_t
typedef struct mooncl_ud_s ud_t;

struct mooncl_ud_s {
    void *handle; /* the object handle bound to this userdata */
    int (*destructor)(lua_State *L, ud_t *ud);  /* self destructor */
    cl_platform platform;
    cl_device device;
    cl_context context;
    cl_program program;     
    ud_t *parent_ud; /* the ud of the parent object */
    mooncl_extdt_t *clext; /* extensions dispatch table */
    uint32_t marks;
    void *info; /* object specific info (ud_info_t, subject to Free() at destruction, if not NULL) */
};
    
/* Marks.  m_ = marks word (uint32_t) , i_ = bit number (0 .. 31)  */
#define MarkGet(m_,i_)  (((m_) & ((uint32_t)1<<(i_))) == ((uint32_t)1<<(i_)))
#define MarkSet(m_,i_)  do { (m_) = ((m_) | ((uint32_t)1<<(i_))); } while(0)
#define MarkReset(m_,i_) do { (m_) = ((m_) & (~((uint32_t)1<<(i_)))); } while(0)

#define IsValid(ud)             MarkGet((ud)->marks, 0)
#define MarkValid(ud)           MarkSet((ud)->marks, 0) 
#define CancelValid(ud)         MarkReset((ud)->marks, 0)

#define HasSubDevices(ud)       MarkGet((ud)->marks, 1)
#define MarkHasSubDevices(ud)   MarkSet((ud)->marks, 1) 
#define CancelHasSubDevices(ud) MarkReset((ud)->marks, 1)

#define IsSubDevice(ud)         MarkGet((ud)->marks, 2)
#define MarkSubDevice(ud)       MarkSet((ud)->marks, 2) 
#define CancelSubDevice(ud)     MarkReset((ud)->marks, 2)

#define IsSubBuffer(ud)         MarkGet((ud)->marks, 3)
#define MarkSubBuffer(ud)       MarkSet((ud)->marks, 3) 
#define CancelSubBuffer(ud)     MarkReset((ud)->marks, 3)

#define IsBufferRegion(ud)      MarkGet((ud)->marks, 4)
#define MarkBufferRegion(ud)    MarkSet((ud)->marks, 4) 
#define CancelBufferRegion(ud)  MarkReset((ud)->marks, 4)

#define IsAllocated(ud)         MarkGet((ud)->marks, 5)
#define MarkAllocated(ud)       MarkSet((ud)->marks, 5) 
#define CancelAllocated(ud)     MarkReset((ud)->marks, 5)

#define IsSvmDontFree(ud)       MarkGet((ud)->marks, 6)
#define MarkSvmDontFree(ud)     MarkSet((ud)->marks, 6) 
#define CancelSvmDontFree(ud)   MarkReset((ud)->marks, 6)

#define IsGLBuffer(ud)          MarkGet((ud)->marks, 7)
#define MarkGLBuffer(ud)        MarkSet((ud)->marks, 7) 
#define CancelGLBuffer(ud)      MarkReset((ud)->marks, 7)

#define IsGLTexture(ud)         MarkGet((ud)->marks, 8)
#define MarkGLTexture(ud)       MarkSet((ud)->marks, 8) 
#define CancelGLTexture(ud)     MarkReset((ud)->marks, 8)

#define IsGLRenderbuffer(ud)       MarkGet((ud)->marks, 9)
#define MarkGLRenderbuffer(ud)     MarkSet((ud)->marks, 9) 
#define CancelGLRenderbuffer(ud)   MarkReset((ud)->marks, 9)

#define IsGLObject(ud)  (IsGLBuffer(ud) || IsGLTexture(ud) || IsGLRenderbuffer(ud))


#if 0
/* .c */
#define  mooncl_
#endif

#define newuserdata mooncl_newuserdata
ud_t *newuserdata(lua_State *L, void *handle, const char *mt, const char *tracename);
#define freeuserdata mooncl_freeuserdata
int freeuserdata(lua_State *L, ud_t *ud, const char *tracename);
#define pushuserdata mooncl_pushuserdata 
int pushuserdata(lua_State *L, ud_t *ud);

#define userdata_unref(L, handle) udata_unref((L),(handle))

#define UD(handle) userdata((handle)) /* dispatchable objects only */
#define userdata mooncl_userdata
ud_t *userdata(void *handle);
#define testxxx mooncl_testxxx
void *testxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define checkxxx mooncl_checkxxx
void *checkxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define pushxxx mooncl_pushxxx
int pushxxx(lua_State *L, void *handle);
#define checkxxxlist mooncl_checkxxxlist
void** checkxxxlist(lua_State *L, int arg, cl_uint *count, int *err, const char *mt);

#define freechildren mooncl_freechildren
int freechildren(lua_State *L,  const char *mt, ud_t *parent_ud);

/* platform.c (dispatchable) */
#define checkplatform(L, arg, udp) (cl_platform)checkxxx((L), (arg), (udp), PLATFORM_MT)
#define testplatform(L, arg, udp) (cl_platform)testxxx((L), (arg), (udp), PLATFORM_MT)
#define pushplatform(L, handle) pushxxx((L), (handle))

/* device.c */
#define createcontextdevices mooncl_createcontextdevices
int createcontextdevices(lua_State *L, cl_platform platform, cl_context context);
#define checkdevice(L, arg, udp) (cl_device)checkxxx((L), (arg), (udp), DEVICE_MT)
#define testdevice(L, arg, udp) (cl_device)testxxx((L), (arg), (udp), DEVICE_MT)
#define pushdevice(L, handle) pushxxx((L), (handle))
#define checkdevicelist(L, arg, count, err) (cl_device*)checkxxxlist((L), (arg), (count), (err), DEVICE_MT)

/* context.c */
#define checkcontext(L, arg, udp) (cl_context)checkxxx((L), (arg), (udp), CONTEXT_MT)
#define testcontext(L, arg, udp) (cl_context)testxxx((L), (arg), (udp), CONTEXT_MT)
#define pushcontext(L, handle) pushxxx((L), (handle))

/* queue.c */
#define checkqueue(L, arg, udp) (cl_queue)checkxxx((L), (arg), (udp), COMMAND_QUEUE_MT)
#define testqueue(L, arg, udp) (cl_queue)testxxx((L), (arg), (udp), COMMAND_QUEUE_MT)
#define pushqueue(L, handle) pushxxx((L), (handle))
#define checkqueuelist(L, arg, count, err) (cl_queue*)checkxxxlist((L), (arg), (count), (err), COMMAND_QUEUE_MT)

/* mem.c (buffer, image or pipe) */
#define testmemobject mooncl_testmemobject
cl_mem testmemobject(lua_State *L, int arg, ud_t **udp);
#define checkmemobject mooncl_checkmemobject
cl_mem checkmemobject(lua_State *L, int arg, ud_t **udp);
#define checkmemobjectlist mooncl_checkmemobjectlist
cl_mem* checkmemobjectlist(lua_State *L, int arg, cl_uint *count, int *err);

/* buffer.c */
#define checkbuffer(L, arg, udp) (cl_buffer)checkxxx((L), (arg), (udp), BUFFER_MT)
#define testbuffer(L, arg, udp) (cl_buffer)testxxx((L), (arg), (udp), BUFFER_MT)
#define pushbuffer(L, handle) pushxxx((L), (handle))
#define checkbufferlist(L, arg, count, err) (cl_buffer*)checkxxxlist((L), (arg), (count), (err), BUFFER_MT)
#define testbufferboundaries mooncl_testbufferboundaries
int testbufferboundaries(lua_State *L, cl_buffer buffer, size_t offset, size_t size);
#define checkbufferboundaries mooncl_checkbufferboundaries
int checkbufferboundaries(lua_State *L, cl_buffer buffer, size_t offset, size_t size);

/* image.c */
#define checkimage(L, arg, udp) (cl_image)checkxxx((L), (arg), (udp), IMAGE_MT)
#define testimage(L, arg, udp) (cl_image)testxxx((L), (arg), (udp), IMAGE_MT)
#define pushimage(L, handle) pushxxx((L), (handle))
#define checkimagelist(L, arg, count, err) (cl_image*)checkxxxlist((L), (arg), (count), (err), IMAGE_MT)

/* pipe.c */
#define checkpipe(L, arg, udp) (cl_pipe)checkxxx((L), (arg), (udp), PIPE_MT)
#define testpipe(L, arg, udp) (cl_pipe)testxxx((L), (arg), (udp), PIPE_MT)
#define pushpipe(L, handle) pushxxx((L), (handle))
#define checkpipelist(L, arg, count, err) (cl_pipe*)checkxxxlist((L), (arg), (count), (err), PIPE_MT)

/* sampler.c */
#define checksampler(L, arg, udp) (cl_sampler)checkxxx((L), (arg), (udp), SAMPLER_MT)
#define testsampler(L, arg, udp) (cl_sampler)testxxx((L), (arg), (udp), SAMPLER_MT)
#define pushsampler(L, handle) pushxxx((L), (handle))
#define checksamplerlist(L, arg, count, err) (cl_sampler*)checkxxxlist((L), (arg), (count), (err), SAMPLER_MT)

/* program.c */
#define checkprogram(L, arg, udp) (cl_program)checkxxx((L), (arg), (udp), PROGRAM_MT)
#define testprogram(L, arg, udp) (cl_program)testxxx((L), (arg), (udp), PROGRAM_MT)
#define pushprogram(L, handle) pushxxx((L), (handle))
#define checkprogramlist(L, arg, count, err) (cl_program*)checkxxxlist((L), (arg), (count), (err), PROGRAM_MT)

/* kernel.c */
#define checkkernel(L, arg, udp) (cl_kernel)checkxxx((L), (arg), (udp), KERNEL_MT)
#define testkernel(L, arg, udp) (cl_kernel)testxxx((L), (arg), (udp), KERNEL_MT)
#define pushkernel(L, handle) pushxxx((L), (handle))
#define checkkernellist(L, arg, count, err) (cl_kernel*)checkxxxlist((L), (arg), (count), (err), KERNEL_MT)

/* event.c */
#define checkevent(L, arg, udp) (cl_event)checkxxx((L), (arg), (udp), EVENT_MT)
#define testevent(L, arg, udp) (cl_event)testxxx((L), (arg), (udp), EVENT_MT)
#define pushevent(L, handle) pushxxx((L), (handle))
#define checkeventlist(L, arg, count, err) (cl_event*)checkxxxlist((L), (arg), (count), (err), EVENT_MT)
#define newevent mooncl_newevent
int newevent(lua_State *L, cl_context context, cl_event event);

/* svm.c */
#define checksvm(L, arg, udp) (cl_svm)checkxxx((L), (arg), (udp), SVM_MT)
#define testsvm(L, arg, udp) (cl_svm)testxxx((L), (arg), (udp), SVM_MT)
#define pushsvm(L, handle) pushxxx((L), (handle))
#define checksvmlist(L, arg, count, err) (cl_svm*)checkxxxlist((L), (arg), (count), (err), SVM_MT)

/* hostmem.c */
#define checkhostmem(L, arg, udp) (cl_hostmem)checkxxx((L), (arg), (udp), HOSTMEM_MT)
#define testhostmem(L, arg, udp) (cl_hostmem)testxxx((L), (arg), (udp), HOSTMEM_MT)
#define pushhostmem(L, handle) pushxxx((L), (handle))
#define checkhostmemlist(L, arg, count, err) (cl_hostmem*)checkxxxlist((L), (arg), (count), (err), HOSTMEM_MT)

/* used in main.c */
void mooncl_open_platform(lua_State *L);
void mooncl_open_device(lua_State *L);
void mooncl_open_context(lua_State *L);
void mooncl_open_queue(lua_State *L);
void mooncl_open_mem(lua_State *L);
void mooncl_open_buffer(lua_State *L);
void mooncl_open_image(lua_State *L);
void mooncl_open_pipe(lua_State *L);
void mooncl_open_gl_object(lua_State *L);
void mooncl_open_sampler(lua_State *L);
void mooncl_open_program(lua_State *L);
void mooncl_open_kernel(lua_State *L);
void mooncl_open_event(lua_State *L);
void mooncl_open_enqueue(lua_State *L);
void mooncl_open_svm(lua_State *L);
void mooncl_open_hostmem(lua_State *L);

#define RAW_FUNC(xxx)                       \
static int Raw(lua_State *L)                \
    {                                       \
    lua_pushinteger(L, (uintptr_t)check##xxx(L, 1, NULL));  \
    return 1;                               \
    }

#define TYPE_FUNC(xxx) /* NONCL */          \
static int Type(lua_State *L)               \
    {                                       \
    (void)check##xxx(L, 1, NULL);           \
    lua_pushstring(L, ""#xxx);              \
    return 1;                               \
    }

#define DELETE_FUNC(xxx)                    \
static int Delete(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)test##xxx(L, 1, &ud);             \
    if(!ud) return 0; /* already deleted */ \
    return ud->destructor(L, ud);           \
    }

#define PARENT_FUNC(xxx)                    \
static int Parent(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->parent_ud) return 0;            \
    return pushuserdata(L, ud->parent_ud);  \
    }

#define PLATFORM_FUNC(xxx)                  \
static int Platform(lua_State *L)           \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->platform) return 0;             \
    return pushplatform(L, ud->platform);   \
    }

#define CONTEXT_FUNC(xxx)                   \
static int Context(lua_State *L)            \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->context) return 0;              \
    return pushcontext(L, ud->context);     \
    }

#define DEVICE_FUNC(xxx)                    \
static int Device(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->device) return 0;               \
    return pushdevice(L, ud->device);       \
    }

#define PROGRAM_FUNC(xxx)                   \
static int Program(lua_State *L)            \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->program) return 0;              \
    return pushprogram(L, ud->program);     \
    }

#define RETAIN_FUNC(xxx, Xxx)               \
static int Retain(lua_State *L)             \
    {                                       \
    cl_##xxx obj = check##xxx(L, 1, NULL);  \
    cl_int ec = cl.Retain##Xxx(obj);         \
    CheckError(L, ec);                      \
    return 0;                               \
    }

#define RELEASE_FUNC(xxx, Xxx, XXX)                 \
static int Release(lua_State *L)                    \
    {                                               \
    cl_int ec;                                      \
    cl_uint refcnt = 0;                             \
    ud_t *ud;                                       \
    cl_##xxx obj = check##xxx(L, 1, &ud);           \
    cl.Get##Xxx##Info(obj, CL_##XXX##_REFERENCE_COUNT, sizeof(refcnt), &refcnt, NULL); \
    if(refcnt==1)                                   \
        return ud->destructor(L, ud);               \
    ec = cl.Release##Xxx(obj);                       \
    CheckError(L, ec);                              \
    return 0;                                       \
    }

#define ReleaseAll(Xxx, XXX, obj) do {                                  \
    cl_uint refcnt_ = 0;                                                \
    cl.Get##Xxx##Info(obj, CL_##XXX##_REFERENCE_COUNT, sizeof(refcnt_), &refcnt_, NULL); \
    /* DBG(""#XXX" refcnt = %u\n", refcnt_); */                         \
    while(refcnt_ > 0)                                                  \
        {   cl.Release##Xxx(obj); refcnt_--; }                           \
} while(0)

/* 'get info' templates ------------------------------------------------------ */

#define GET_INFO_STRING(Func, Xxx, xxx, xxxinfo)                        \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    cl_int ec;                                                          \
    char *value;                                                        \
    size_t size;                                                        \
                                                                        \
    ec = cl.Get##Xxx##Info(obj, name, 0, NULL, &size);                   \
    CheckError(L, ec);                                                  \
                                                                        \
    if(size==0)                                                         \
        { lua_pushstring(L, ""); return 1; }                            \
                                                                        \
    value = (char*)Malloc(L, size);                                     \
    ec = cl.Get##Xxx##Info(obj, name, size, value, NULL);                \
    if(ec)                                                              \
        {                                                               \
        Free(L, value);                                                 \
        CheckError(L, ec);                                              \
        return 0;                                                       \
        }                                                               \
    if(size>0 && value[size-1]=='\0') size--; /* strip trailing NIL */  \
    if(size==0)                                                         \
        lua_pushstring(L, "");                                          \
    else                                                                \
        lua_pushlstring(L, value, size);                                \
    Free(L, value);                                                     \
    return 1;                                                           \
    }

#define GET_INFO_BOOLEAN(Func, Xxx, xxx, xxxinfo)                       \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    cl_bool value;                                                      \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    lua_pushboolean(L, value);                                          \
    return 1;                                                           \
    }

#define GET_INFO_INTEGER(Func, Xxx, xxx, xxxinfo, T)                    \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    T value;                                                            \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    lua_pushinteger(L, value);                                          \
    return 1;                                                           \
    }

#define GET_INFO_UINT(Func, Xxx, xxx, xxxinfo) GET_INFO_INTEGER(Func, Xxx, xxx, xxxinfo, cl_uint)
#define GET_INFO_ULONG(Func, Xxx, xxx, xxxinfo) GET_INFO_INTEGER(Func, Xxx, xxx, xxxinfo, cl_ulong)
#define GET_INFO_SIZE(Func, Xxx, xxx, xxxinfo) GET_INFO_INTEGER(Func, Xxx, xxx, xxxinfo, size_t)
#define GET_INFO_FLAGS(Func, Xxx, xxx, xxxinfo) GET_INFO_INTEGER(Func, Xxx, xxx, xxxinfo, cl_bitfield)

#define GET_INFO_NUMBER(Func, Xxx, xxx, xxxinfo, T)                     \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    T value;                                                            \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    lua_pushnumber(L, value);                                           \
    return 1;                                                           \
    }

#define GET_INFO_FLOAT(Func, Xxx, xxx, xxxinfo) GET_INFO_NUMBER(Func, Xxx, xxx, xxxinfo, float)
#define GET_INFO_DOUBLE(Func, Xxx, xxx, xxxinfo) GET_INFO_NUMBER(Func, Xxx, xxx, xxxinfo, double)

#define GET_INFO_OBJECT(Func, Xxx, xxx, xxxinfo, objtype)               \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    cl_##objtype value;                                                 \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    if(value == 0) return 0;                                            \
    push##objtype(L, value);                                            \
    return 1;                                                           \
    }

#define GET_INFO_ENUM(Func, Xxx, xxx, xxxinfo, pushfunc)                \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    cl_uint value;                                                      \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    pushfunc(L, value);                                                 \
    return 1;                                                           \
    }

#define GET_INFO_STRUCT(Func, Xxx, xxx, xxxinfo, structtype, pushfunc)  \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                                   \
    structtype value;                                                   \
    cl_int ec = cl.Get##Xxx##Info(obj, name, sizeof(value), &value, NULL);\
    CheckError(L, ec);                                                  \
    pushfunc(L, &value);                                                \
    return 1;                                                           \
    }

#define GET_INFO_DEVICES(Func, Xxx, xxx, xxxinfo)       \
static int Func(lua_State *L, cl_##xxx obj, cl_##xxxinfo##_info name)   \
    {                                                   \
    cl_int ec;                                          \
    cl_device *value;                                   \
    size_t size, n, i;                                  \
                                                        \
    ec = cl.Get##Xxx##Info(obj, name, 0, NULL, &size);   \
    CheckError(L, ec);                                  \
                                                        \
    lua_newtable(L);                                    \
    if(size == 0) return 1;                             \
                                                        \
    n = size / sizeof(cl_device);                       \
    value = (cl_device*)Malloc(L, n*sizeof(cl_device)); \
    ec = cl.Get##Xxx##Info(obj, name, size, value, NULL);\
    if(ec)                                              \
        {                                               \
        Free(L, value);                                 \
        CheckError(L, ec);                              \
        return 0;                                       \
        }                                               \
                                                        \
    for(i=0; i < n; i++)                                \
        {                                               \
        if(value[i]==0) continue;                       \
        pushdevice(L, value[i]);                        \
        lua_rawseti(L, -2, i+1);                        \
        }                                               \
    Free(L, value);                                     \
    return 1;                                           \
    }

#endif /* objectsDEFINED */
