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

/********************************************************************************
 * Internal common header                                                       *
 ********************************************************************************/

#ifndef internalDEFINED
#define internalDEFINED

#ifdef LINUX
#define _ISOC11_SOURCE /* see man aligned_alloc(3) */
#endif
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mooncl.h"
#include "getproc.h"

#define TOSTR_(x) #x
#define TOSTR(x) TOSTR_(x)

#include "tree.h"
#include "objects.h"
#include "enums.h"

/* Note: all the dynamic symbols of this library (should) start with 'mooncl_' .
 * The only exception is the luaopen_mooncl() function, which is searched for
 * with that name by Lua.
 * MoonCL's string references on the Lua registry also start with 'mooncl_'.
 */


#if 0
/* .c */
#define  mooncl_
#endif

/* utils.c */
#define noprintf mooncl_noprintf
int noprintf(const char *fmt, ...); 
#define is_little_endian mooncl_is_little_endian
cl_bool is_little_endian(void);
#define now mooncl_now
double now(void);
#define sleeep mooncl_sleeep
void sleeep(double seconds);
#define since(t) (now() - (t))
#define notavailable mooncl_notavailable
int notavailable(lua_State *L, ...);
#define Malloc mooncl_Malloc
void *Malloc(lua_State *L, size_t size);
#define MallocNoErr mooncl_MallocNoErr
void *MallocNoErr(lua_State *L, size_t size);
#define Strdup mooncl_Strdup
char *Strdup(lua_State *L, const char *s);
#define Free mooncl_Free
void Free(lua_State *L, void *ptr);
#define checkboolean mooncl_checkboolean
int checkboolean(lua_State *L, int arg);
#define testboolean mooncl_testboolean
int testboolean(lua_State *L, int arg, int *err);
#define optboolean mooncl_optboolean
int optboolean(lua_State *L, int arg, int d);
#define checklightuserdata mooncl_checklightuserdata
void *checklightuserdata(lua_State *L, int arg);
#define checklightuserdataorzero mooncl_checklightuserdataorzero
void *checklightuserdataorzero(lua_State *L, int arg);
#define optlightuserdata mooncl_optlightuserdata
void *optlightuserdata(lua_State *L, int arg);
#if 0
#define testindex mooncl_testindex
int testindex(lua_State *L, int arg, int *err);
#define checkindex mooncl_checkindex
int checkindex(lua_State *L, int arg);
#define optindex mooncl_optindex
int optindex(lua_State *L, int arg, int optval);
#define pushindex mooncl_pushindex
void pushindex(lua_State *L, int val);
#endif

/* lists.c */
#define checkstringlist mooncl_checkstringlist
char** checkstringlist(lua_State *L, int arg, cl_uint *count, int *err);
#define freestringlist mooncl_freestringlist
void freestringlist(lua_State *L, char** list, cl_uint count);
#define pushstringlist mooncl_pushstringlist
void pushstringlist(lua_State *L, char** list, cl_uint count);
#define pusherrcode mooncl_pusherrcode
int pusherrcode(lua_State *L, cl_int ec);
#define checklstringlist mooncl_checklstringlist
char** checklstringlist(lua_State *L, int arg, cl_uint *count, size_t **lengths_, int *err);
#define freelstringlist mooncl_freelstringlist
void freelstringlist(lua_State *L, char** list, size_t *lengths, cl_uint count);
#define pushlstringlist mooncl_pushlstringlist
void pushlstringlist(lua_State *L, char** list, size_t *lengths, cl_uint count);
#define checksizelist mooncl_checksizelist
size_t* checksizelist(lua_State *L, int arg, cl_uint *count, int *err);
#define pushsizelist mooncl_pushsizelist
void pushsizelist(lua_State *L, size_t *list, cl_uint count);
#define checksize3 mooncl_checksize3
int checksize3(lua_State *L, int arg, size_t dst[3]);
#define pushsize3 mooncl_pushsize3
void pushsize3(lua_State *L, size_t list[3]);
#define checklightuserdatalist mooncl_checklightuserdatalist
void** checklightuserdatalist(lua_State *L, int arg, cl_uint *count, int *err);
#define pushlightuserdatalist mooncl_pushlightuserdatalist
void pushlightuserdatalist(lua_State *L, void* *list, cl_uint count);

/* datahandling.c */
#define sizeofprimtype mooncl_sizeofprimtype
size_t sizeofprimtype(int type);
#define toflattable mooncl_toflattable
int toflattable(lua_State *L, int arg);
#define testdata mooncl_testdata
int testdata(lua_State *L, int type, size_t n, void *dst, size_t dstsize);
#define checkdata mooncl_checkdata
int checkdata(lua_State *L, int arg, int type, void *dts, size_t dstsize);
#define pushdata mooncl_pushdata
int pushdata(lua_State *L, int type, void *src, size_t srcsize);

#define checkflags(L, arg) (cl_bitfield)luaL_checkinteger((L), (arg))
#define pushflags(L, val) lua_pushinteger((L), (val))

/* Internal error codes */
#define ERR_NOTPRESENT       1
#define ERR_SUCCESS          0
#define ERR_GENERIC         -1
#define ERR_TYPE            -2
#define ERR_VALUE           -3
#define ERR_TABLE           -4
#define ERR_EMPTY           -5
#define ERR_MEMORY          -6
#define ERR_MALLOC_ZERO     -7
#define ERR_LENGTH          -8
#define ERR_POOL            -9
#define ERR_BOUNDARIES      -10
#define ERR_UNKNOWN         -11
#define errstring mooncl_errstring
const char* errstring(int err);

/* tracing.c */
#define trace_objects mooncl_trace_objects
extern int trace_objects;

/* structs.c */
#define echeckdevicepartitionproperty mooncl_echeckdevicepartitionproperty
cl_device_partition_property *echeckdevicepartitionproperty(lua_State *L, int arg, int *err);
#define pushdevicepartitionproperty mooncl_pushdevicepartitionproperty
int pushdevicepartitionproperty(lua_State *L, cl_device_partition_property *p);
#define echeckcontextproperties mooncl_echeckcontextproperties
cl_context_properties *echeckcontextproperties(lua_State *L, int arg, int *err, cl_platform use_platform);
#define pushcontextproperties mooncl_pushcontextproperties
int pushcontextproperties(lua_State *L, cl_context_properties *p);
#define echeckqueueproperties mooncl_echeckqueueproperties
cl_queue_properties *echeckqueueproperties(lua_State *L, int arg, cl_command_queue_properties *propflags, int *err);
#define pushqueueproperties mooncl_pushqueueproperties
int pushqueueproperties(lua_State *L, cl_queue_properties *p);
#define echecksamplerproperties mooncl_echecksamplerproperties
cl_sampler_properties *echecksamplerproperties(lua_State *L, int arg, cl_bool *v1_2_normalized_coords, cl_addressing_mode *v1_2_addressing_mode,  cl_filter_mode *v1_2_filter_mode, int *err);
#define pushsamplerproperties mooncl_pushsamplerproperties
int pushsamplerproperties(lua_State *L, cl_sampler_properties *p);
#define echeckpipeproperties mooncl_echeckpipeproperties
cl_pipe_properties *echeckpipeproperties(lua_State *L, int arg, int *err);
#define pushpipeproperties mooncl_pushpipeproperties
int pushpipeproperties(lua_State *L, cl_pipe_properties *p);
#define echeckimageformat mooncl_echeckimageformat
int echeckimageformat(lua_State *L, int arg, cl_image_format *p);
#define pushimageformat mooncl_pushimageformat
int pushimageformat(lua_State *L, cl_image_format *p);
#define echeckimagedesc mooncl_echeckimagedesc
int echeckimagedesc(lua_State *L, int arg, cl_image_desc *p);
#define pushimagedesc mooncl_pushimagedesc
int pushimagedesc(lua_State *L, cl_image_desc *p);

/* getproc.c */
void mooncl_atexit_getproc(void);
int mooncl_open_getproc(lua_State *L);
mooncl_extdt_t *mooncl_getproc_extensions(lua_State *L, cl_platform platform);

/* main.c */
int luaopen_mooncl(lua_State *L);
void mooncl_utils_init(lua_State *L);
void mooncl_open_versions(lua_State *L);
void mooncl_open_enums(lua_State *L);
void mooncl_open_flags(lua_State *L);
void mooncl_open_tracing(lua_State *L);
void mooncl_open_datahandling(lua_State *L);

/*------------------------------------------------------------------------------*
 | Debug and other utilities                                                    |
 *------------------------------------------------------------------------------*/

#define CheckError(L, ec) \
    do { if(ec != CL_SUCCESS) { pusherrcode((L), (ec)); return lua_error(L); } } while(0)

/* If this is printed, it denotes a suspect bug: */
#define UNEXPECTED_ERROR "unexpected error (%s, %d)", __FILE__, __LINE__
#define unexpected(L) luaL_error((L), UNEXPECTED_ERROR)

#define notsupported(L) luaL_error((L), "operation not supported")

#define badvalue(L,s)   lua_pushfstring((L), "invalid value '%s'", (s))

/* DEBUG -------------------------------------------------------- */
#if defined(DEBUG)

#define DBG printf
#define TR() do { printf("trace %s %d\n",__FILE__,__LINE__); } while(0)
#define BK() do { printf("break %s %d\n",__FILE__,__LINE__); getchar(); } while(0)
#define TSTART double ts = now();
#define TSTOP do {                                          \
    ts = since(ts); ts = ts*1e6;                            \
    printf("%s %d %.3f us\n", __FILE__, __LINE__, ts);      \
    ts = now();                                             \
} while(0);

#else 

#define DBG noprintf
#define TR()
#define BK()
#define TSTART do {} while(0) 
#define TSTOP do {} while(0)    

#endif /* DEBUG ------------------------------------------------- */


#endif /* internalDEFINED */
