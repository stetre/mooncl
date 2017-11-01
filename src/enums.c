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
 | Code<->string map for enumerations                                           |
 *------------------------------------------------------------------------------*/


/* code <-> string record */
#define rec_t struct rec_s
struct rec_s {
    RB_ENTRY(rec_s) CodeEntry;
    RB_ENTRY(rec_s) StringEntry;
    uint32_t domain;
    uint32_t code;  /* (domain, code) = search key in code tree */
    char     *str;  /* (domain, str) = search key in string tree */
};

/* compare functions */
static int cmp_code(rec_t *rec1, rec_t *rec2) 
    { 
    if(rec1->domain != rec2->domain)
        return (rec1->domain < rec2->domain ? -1 : rec1->domain > rec2->domain);
    return (rec1->code < rec2->code ? -1 : rec1->code > rec2->code);
    } 

static int cmp_str(rec_t *rec1, rec_t *rec2) 
    { 
    if(rec1->domain != rec2->domain)
        return (rec1->domain < rec2->domain ? -1 : rec1->domain > rec2->domain);
    return strcmp(rec1->str, rec2->str);
    } 

static RB_HEAD(CodeTree, rec_s) CodeHead = RB_INITIALIZER(&CodeHead);
RB_PROTOTYPE_STATIC(CodeTree, rec_s, CodeEntry, cmp_code) 
RB_GENERATE_STATIC(CodeTree, rec_s, CodeEntry, cmp_code) 

static RB_HEAD(StringTree, rec_s) StringHead = RB_INITIALIZER(&StringHead);
RB_PROTOTYPE_STATIC(StringTree, rec_s, StringEntry, cmp_str) 
RB_GENERATE_STATIC(StringTree, rec_s, StringEntry, cmp_str) 
 
static rec_t *code_remove(rec_t *rec) 
    { return RB_REMOVE(CodeTree, &CodeHead, rec); }
static rec_t *code_insert(rec_t *rec) 
    { return RB_INSERT(CodeTree, &CodeHead, rec); }
static rec_t *code_search(uint32_t domain, uint32_t code) 
    { rec_t tmp; tmp.domain = domain; tmp.code = code; return RB_FIND(CodeTree, &CodeHead, &tmp); }
static rec_t *code_first(uint32_t domain, uint32_t code) 
    { rec_t tmp; tmp.domain = domain; tmp.code = code; return RB_NFIND(CodeTree, &CodeHead, &tmp); }
static rec_t *code_next(rec_t *rec)
    { return RB_NEXT(CodeTree, &CodeHead, rec); }
#if 0
static rec_t *code_prev(rec_t *rec)
    { return RB_PREV(CodeTree, &CodeHead, rec); }
static rec_t *code_min(void)
    { return RB_MIN(CodeTree, &CodeHead); }
static rec_t *code_max(void)
    { return RB_MAX(CodeTree, &CodeHead); }
static rec_t *code_root(void)
    { return RB_ROOT(&CodeHead); }
#endif
 
static rec_t *str_remove(rec_t *rec) 
    { return RB_REMOVE(StringTree, &StringHead, rec); }
static rec_t *str_insert(rec_t *rec) 
    { return RB_INSERT(StringTree, &StringHead, rec); }
static rec_t *str_search(uint32_t domain, const char* str) 
    { rec_t tmp; tmp.domain = domain; tmp.str = (char*)str; return RB_FIND(StringTree, &StringHead, &tmp); }
#if 0
static rec_t *str_first(uint32_t domain, const char* str ) 
    { rec_t tmp; tmp.domain = domain; tmp.str = str; return RB_NFIND(StringTree, &StringHead, &tmp); }
static rec_t *str_next(rec_t *rec)
    { return RB_NEXT(StringTree, &StringHead, rec); }
static rec_t *str_prev(rec_t *rec)
    { return RB_PREV(StringTree, &StringHead, rec); }
static rec_t *str_min(void)
    { return RB_MIN(StringTree, &StringHead); }
static rec_t *str_max(void)
    { return RB_MAX(StringTree, &StringHead); }
static rec_t *str_root(void)
    { return RB_ROOT(&StringHead); }
#endif


static int enums_new(lua_State *L, uint32_t domain, uint32_t code, const char *str)
    {
    rec_t *rec;
    if((rec = (rec_t*)Malloc(L, sizeof(rec_t))) == NULL) 
        return luaL_error(L, errstring(ERR_MEMORY));

    memset(rec, 0, sizeof(rec_t));
    rec->domain = domain;
    rec->code = code;
    rec->str = Strdup(L, str);
    if(code_search(domain, code) || str_search(domain, str))
        { 
        Free(L, rec->str);
        Free(L, rec); 
        return unexpected(L); /* duplicate value */
        }
    code_insert(rec);
    str_insert(rec);
    return 0;
    }

static void enums_free(lua_State *L, rec_t* rec)
    {
    if(code_search(rec->domain, rec->code) == rec)
        code_remove(rec);
    if(str_search(rec->domain, rec->str) == rec)
        str_remove(rec);
    Free(L, rec->str);
    Free(L, rec);   
    }

void enums_free_all(lua_State *L)
    {
    rec_t *rec;
    while((rec = code_first(0, 0)))
        enums_free(L, rec);
    }

#if 0
uint32_t enums_code(uint32_t domain, const char *str, int* found)
    {
    rec_t *rec = str_search(domain, str);
    if(!rec)
        { *found = 0; return 0; }
    *found = 1; 
    return rec->code;
    }

const char* enums_string(uint32_t domain, uint32_t code)
    {
    rec_t *rec = code_search(domain, code);
    if(!rec)
        return NULL;
    return rec->str;
    }

#endif


uint32_t enums_test(lua_State *L, uint32_t domain, int arg, int *err)
    {
    rec_t *rec;
    const char *s = luaL_optstring(L, arg, NULL);

    if(!s)
        { *err = ERR_NOTPRESENT; return 0; }

    rec = str_search(domain, s);
    if(!rec)
        { *err = ERR_VALUE; return 0; }
    
    *err = ERR_SUCCESS;
    return rec->code;
    }

uint32_t enums_check(lua_State *L, uint32_t domain, int arg)
    {
    rec_t *rec;
    const char *s = luaL_checkstring(L, arg);

    rec = str_search(domain, s);
    if(!rec)
        return luaL_argerror(L, arg, badvalue(L, s));
    
    return rec->code;
    }

int enums_push(lua_State *L, uint32_t domain, uint32_t code)
    {
    rec_t *rec = code_search(domain, code);

    if(!rec)
        return unexpected(L);

    lua_pushstring(L, rec->str);
    return 1;
    }

int enums_values(lua_State *L, uint32_t domain)
    {
    int i;
    rec_t *rec;

    lua_newtable(L);
    i = 1;
    rec = code_first(domain, 0);
    while(rec)
        {
        if(rec->domain == domain)
            {
            lua_pushstring(L, rec->str);
            lua_rawseti(L, -2, i++);
            }
        rec = code_next(rec);
        }

    return 1;
    }


uint32_t* enums_checklist(lua_State *L, uint32_t domain, int arg, uint32_t *count, int *err)
    {
    uint32_t* list;
    uint32_t i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_NOTPRESENT; return NULL; }

    list = (uint32_t*)MallocNoErr(L, sizeof(uint32_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        list[i] = enums_test(L, domain, -1, err);
        lua_pop(L, 1);
        if(*err)
            { Free(L, list); *count = 0; return NULL; }
        }
    return list;
    }

void enums_freelist(lua_State *L, uint32_t *list)
    {
    if(!list)
        return;
    Free(L, list);
    }

/*------------------------------------------------------------------------------*
 |                                                                              |
 *------------------------------------------------------------------------------*/

static int Enum(lua_State *L)
/* { strings } = cl.enum('type') lists all the values for a given enum type */
    { 
    const char *s = luaL_checkstring(L, 1); 
#define CASE(xxx) if(strcmp(s, ""#xxx) == 0) return values##xxx(L)
    CASE(platforminfo);
    CASE(deviceinfo);
    CASE(devicememcachetype);
    CASE(devicelocalmemtype);
    CASE(contextinfo);
    CASE(commandqueueinfo);
    CASE(channelorder);
    CASE(channeltype);
    CASE(memobjecttype);
    CASE(meminfo);
    CASE(imageinfo);
    CASE(pipeinfo);
    CASE(addressingmode);
    CASE(filtermode);
    CASE(samplerinfo);
    CASE(programinfo);
    CASE(programbuildinfo);
    CASE(programbinarytype);
    CASE(kernelinfo);
    CASE(kernelarginfo);
    CASE(argaddressqualifier);
    CASE(argaccessqualifier);
    CASE(kernelworkgroupinfo);
    CASE(kernelsubgroupinfo);
    CASE(kernelexecinfo);
    CASE(eventinfo);
    CASE(commandtype);
    CASE(executionstatus);
    CASE(buffercreatetype);
    CASE(profilinginfo);
    CASE(buildstatus);
    CASE(primtype);
    CASE(gltextureinfo);
    CASE(glcontextinfo);
    CASE(queuepriority);
    CASE(queuethrottle);
    CASE(gltexturetarget);
    CASE(globjecttype);
#undef CASE
    return 0;
    }

static const struct luaL_Reg Functions[] = 
    {
        { "enum", Enum },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_enums(lua_State *L)
    {
    uint32_t domain;

    luaL_setfuncs(L, Functions, 0);

    /* Add all the code<->string mappings and the cl.XXX constant strings */
#define ADD(what, s) do { enums_new(L, domain, NONCL_##what, s); } while(0)
    domain = DOMAIN_NONCL_TYPE; 
    ADD(TYPE_CHAR, "char");
    ADD(TYPE_UCHAR, "uchar");
    ADD(TYPE_SHORT, "short");
    ADD(TYPE_USHORT, "ushort");
    ADD(TYPE_INT, "int");
    ADD(TYPE_UINT, "uint");
    ADD(TYPE_LONG, "long");
    ADD(TYPE_ULONG, "ulong");
    ADD(TYPE_HALF, "half");
    ADD(TYPE_FLOAT, "float");
    ADD(TYPE_DOUBLE, "double");
#undef ADD

#define ADD(what, s) do {                               \
    lua_pushstring(L, s); lua_setfield(L, -2, #what);   \
    enums_new(L, domain, CL_##what, s);                 \
} while(0)

    domain = DOMAIN_PLATFORM_INFO; /* cl_platform_info */
    ADD(PLATFORM_PROFILE, "profile");
    ADD(PLATFORM_VERSION, "version");
    ADD(PLATFORM_VENDOR, "vendor");
    ADD(PLATFORM_NAME, "name");
    ADD(PLATFORM_EXTENSIONS, "extensions");
    ADD(PLATFORM_HOST_TIMER_RESOLUTION, "host timer resolution"); //CL_VERSION_2_1
    ADD(PLATFORM_ICD_SUFFIX_KHR, "icd suffix");

    domain = DOMAIN_DEVICE_INFO; /* cl_device_info */
    ADD(DEVICE_TYPE, "type");
    ADD(DEVICE_VENDOR_ID, "vendor id");
    ADD(DEVICE_MAX_COMPUTE_UNITS, "max compute units");
    ADD(DEVICE_MAX_WORK_ITEM_DIMENSIONS, "max work item dimensions");
    ADD(DEVICE_MAX_WORK_GROUP_SIZE, "max work group size");
    ADD(DEVICE_MAX_WORK_ITEM_SIZES, "max work item sizes");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, "preferred vector width char");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, "preferred vector width short");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_INT, "preferred vector width int");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_LONG, "preferred vector width long");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, "preferred vector width float");
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, "preferred vector width double");
    ADD(DEVICE_MAX_CLOCK_FREQUENCY, "max clock frequency");
    ADD(DEVICE_ADDRESS_BITS, "address bits");
    ADD(DEVICE_MAX_READ_IMAGE_ARGS, "max read image args");
    ADD(DEVICE_MAX_WRITE_IMAGE_ARGS, "max write image args");
    ADD(DEVICE_MAX_MEM_ALLOC_SIZE, "max mem alloc size");
    ADD(DEVICE_IMAGE2D_MAX_WIDTH, "image2d max width");
    ADD(DEVICE_IMAGE2D_MAX_HEIGHT, "image2d max height");
    ADD(DEVICE_IMAGE3D_MAX_WIDTH, "image3d max width");
    ADD(DEVICE_IMAGE3D_MAX_HEIGHT, "image3d max height");
    ADD(DEVICE_IMAGE3D_MAX_DEPTH, "image3d max depth");
    ADD(DEVICE_IMAGE_SUPPORT, "image support");
    ADD(DEVICE_MAX_PARAMETER_SIZE, "max parameter size");
    ADD(DEVICE_MAX_SAMPLERS, "max samplers");
    ADD(DEVICE_MEM_BASE_ADDR_ALIGN, "mem base addr align");
    ADD(DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "min data type align size");
    ADD(DEVICE_SINGLE_FP_CONFIG, "single fp config");
    ADD(DEVICE_GLOBAL_MEM_CACHE_TYPE, "global mem cache type");
    ADD(DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "global mem cacheline size");
    ADD(DEVICE_GLOBAL_MEM_CACHE_SIZE, "global mem cache size");
    ADD(DEVICE_GLOBAL_MEM_SIZE, "global mem size");
    ADD(DEVICE_MAX_CONSTANT_BUFFER_SIZE, "max constant buffer size");
    ADD(DEVICE_MAX_CONSTANT_ARGS, "max constant args");
    ADD(DEVICE_LOCAL_MEM_TYPE, "local mem type");
    ADD(DEVICE_LOCAL_MEM_SIZE, "local mem size");
    ADD(DEVICE_ERROR_CORRECTION_SUPPORT, "error correction support");
    ADD(DEVICE_PROFILING_TIMER_RESOLUTION, "profiling timer resolution");
    ADD(DEVICE_ENDIAN_LITTLE, "endian little");
    ADD(DEVICE_AVAILABLE, "available");
    ADD(DEVICE_COMPILER_AVAILABLE, "compiler available");
    ADD(DEVICE_EXECUTION_CAPABILITIES, "execution capabilities");
//  ADD(DEVICE_QUEUE_PROPERTIES, "queue properties");
    ADD(DEVICE_QUEUE_ON_HOST_PROPERTIES, "queue on host properties");
    ADD(DEVICE_NAME, "name");
    ADD(DEVICE_VENDOR, "vendor");
    ADD(DRIVER_VERSION, "driver version");
    ADD(DEVICE_PROFILE, "profile");
    ADD(DEVICE_VERSION, "version");
    ADD(DEVICE_EXTENSIONS, "extensions");
    ADD(DEVICE_PLATFORM, "platform");
    ADD(DEVICE_DOUBLE_FP_CONFIG, "double fp config");
    ADD(DEVICE_HALF_FP_CONFIG, "half fp config"); //CL_VERSION_2_1
    ADD(DEVICE_PREFERRED_VECTOR_WIDTH_HALF, "preferred vector width half");
//  ADD(DEVICE_HOST_UNIFIED_MEMORY, "host unified memory");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_CHAR, "native vector width char");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_SHORT, "native vector width short");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_INT, "native vector width int");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_LONG, "native vector width long");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, "native vector width float");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, "native vector width double");
    ADD(DEVICE_NATIVE_VECTOR_WIDTH_HALF, "native vector width half");
    ADD(DEVICE_OPENCL_C_VERSION, "opencl c version");
    ADD(DEVICE_LINKER_AVAILABLE, "linker available");
    ADD(DEVICE_BUILT_IN_KERNELS, "built in kernels");
    ADD(DEVICE_IMAGE_MAX_BUFFER_SIZE, "image max buffer size");
    ADD(DEVICE_IMAGE_MAX_ARRAY_SIZE, "image max array size");
    ADD(DEVICE_PARENT_DEVICE, "parent device");
    ADD(DEVICE_PARTITION_MAX_SUB_DEVICES, "partition max sub devices");
    ADD(DEVICE_PARTITION_PROPERTIES, "partition properties");
    ADD(DEVICE_PARTITION_AFFINITY_DOMAIN, "partition affinity domain");
    ADD(DEVICE_PARTITION_TYPE, "partition type");
    ADD(DEVICE_REFERENCE_COUNT, "reference count");
    ADD(DEVICE_PREFERRED_INTEROP_USER_SYNC, "preferred interop user sync");
    ADD(DEVICE_PRINTF_BUFFER_SIZE, "printf buffer size");
    ADD(DEVICE_IMAGE_PITCH_ALIGNMENT, "image pitch alignment");
    ADD(DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, "image base address alignment");
    ADD(DEVICE_MAX_READ_WRITE_IMAGE_ARGS, "max read write image args");
    ADD(DEVICE_MAX_GLOBAL_VARIABLE_SIZE, "max global variable size");
    ADD(DEVICE_QUEUE_ON_DEVICE_PROPERTIES, "queue on device properties");
    ADD(DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, "queue on device preferred size");
    ADD(DEVICE_QUEUE_ON_DEVICE_MAX_SIZE, "queue on device max size");
    ADD(DEVICE_MAX_ON_DEVICE_QUEUES, "max on device queues");
    ADD(DEVICE_MAX_ON_DEVICE_EVENTS, "max on device events");
    ADD(DEVICE_SVM_CAPABILITIES, "svm capabilities");
    ADD(DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE, "global variable preferred total size");
    ADD(DEVICE_MAX_PIPE_ARGS, "max pipe args");
    ADD(DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS, "pipe max active reservations");
    ADD(DEVICE_PIPE_MAX_PACKET_SIZE, "pipe max packet size");
    ADD(DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT, "preferred platform atomic alignment");
    ADD(DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT, "preferred global atomic alignment");
    ADD(DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT, "preferred local atomic alignment");
    ADD(DEVICE_IL_VERSION, "il version");// CL_VERSION_2_1
    ADD(DEVICE_MAX_NUM_SUB_GROUPS, "max num sub groups");// CL_VERSION_2_1
    ADD(DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS, "sub group independent forward progress");// CL_VERSION_2_1
    ADD(DEVICE_TERMINATE_CAPABILITY_KHR, "terminate capability");
    ADD(DEVICE_SPIR_VERSIONS, "spir versions");
    ADD(DEVICE_MAX_NAMED_BARRIER_COUNT_KHR, "max named barrier count");

    domain = DOMAIN_DEVICE_MEM_CACHE_TYPE; /* cl_device_mem_cache_type */
    ADD(NONE, "none");
    ADD(READ_ONLY_CACHE, "read only cache");
    ADD(READ_WRITE_CACHE, "read write cache");

    domain = DOMAIN_DEVICE_LOCAL_MEM_TYPE; /* cl_device_local_mem_type */
    ADD(LOCAL, "local");
    ADD(GLOBAL, "global");

    domain = DOMAIN_CONTEXT_INFO; /* cl_context_info  */
    ADD(CONTEXT_REFERENCE_COUNT, "reference count");
    ADD(CONTEXT_DEVICES, "devices");
    ADD(CONTEXT_PROPERTIES, "properties");
    ADD(CONTEXT_NUM_DEVICES, "num devices");

    domain = DOMAIN_COMMAND_QUEUE_INFO; /* cl_command_queue_info */
    ADD(QUEUE_CONTEXT, "context");
    ADD(QUEUE_DEVICE, "device");
    ADD(QUEUE_REFERENCE_COUNT, "reference count");
    ADD(QUEUE_PROPERTIES, "properties");
    ADD(QUEUE_SIZE, "size");
    ADD(QUEUE_DEVICE_DEFAULT, "device default"); //CL_VERSION_2_1

    domain = DOMAIN_CHANNEL_ORDER; /* cl_channel_order */
    ADD(R, "r");
    ADD(A, "a");
    ADD(RG, "rg");
    ADD(RA, "ra");
    ADD(RGB, "rgb");
    ADD(RGBA, "rgba");
    ADD(BGRA, "bgra");
    ADD(ARGB, "argb");
    ADD(INTENSITY, "intensity");
    ADD(LUMINANCE, "luminance");
    ADD(Rx, "rx");
    ADD(RGx, "rgx");
    ADD(RGBx, "rgbx");
    ADD(DEPTH, "depth");
    ADD(DEPTH_STENCIL, "depth stencil");
    ADD(sRGB, "srgb");
    ADD(sRGBx, "srgbx");
    ADD(sRGBA, "srgba");
    ADD(sBGRA, "sbgra");
    ADD(ABGR, "abgr");

    domain = DOMAIN_CHANNEL_TYPE; /* cl_channel_type */
    ADD(SNORM_INT8, "snorm int8");
    ADD(SNORM_INT16, "snorm int16");
    ADD(UNORM_INT8, "unorm int8");
    ADD(UNORM_INT16, "unorm int16");
    ADD(UNORM_SHORT_565, "unorm short 565");
    ADD(UNORM_SHORT_555, "unorm short 555");
    ADD(UNORM_INT_101010, "unorm int 101010");
    ADD(SIGNED_INT8, "signed int8");
    ADD(SIGNED_INT16, "signed int16");
    ADD(SIGNED_INT32, "signed int32");
    ADD(UNSIGNED_INT8, "unsigned int8");
    ADD(UNSIGNED_INT16, "unsigned int16");
    ADD(UNSIGNED_INT32, "unsigned int32");
    ADD(HALF_FLOAT, "half float");
    ADD(FLOAT, "float");
    ADD(UNORM_INT24, "unorm int24");
    ADD(UNORM_INT_101010_2, "unorm int 101010 2"); //CL_VERSION_2_1

    domain = DOMAIN_MEM_OBJECT_TYPE; /* cl_mem_object_type */
    ADD(MEM_OBJECT_BUFFER, "buffer");
    ADD(MEM_OBJECT_IMAGE2D, "image2d");
    ADD(MEM_OBJECT_IMAGE3D, "image3d");
    ADD(MEM_OBJECT_IMAGE2D_ARRAY, "image2d array");
    ADD(MEM_OBJECT_IMAGE1D, "image1d");
    ADD(MEM_OBJECT_IMAGE1D_ARRAY, "image1d array");
    ADD(MEM_OBJECT_IMAGE1D_BUFFER, "image1d buffer");
    ADD(MEM_OBJECT_PIPE, "pipe");

    domain = DOMAIN_MEM_INFO; /* cl_mem_info */
    ADD(MEM_TYPE, "type");
    ADD(MEM_FLAGS, "flags");
    ADD(MEM_SIZE, "size");
    ADD(MEM_HOST_PTR, "host ptr");
    ADD(MEM_MAP_COUNT, "map count");
    ADD(MEM_REFERENCE_COUNT, "reference count");
    ADD(MEM_CONTEXT, "context");
    ADD(MEM_ASSOCIATED_MEMOBJECT, "associated memobject");
    ADD(MEM_OFFSET, "offset");
    ADD(MEM_USES_SVM_POINTER, "uses svm pointer");

    domain = DOMAIN_IMAGE_INFO; /* cl_image_info */
    ADD(IMAGE_FORMAT, "format");
    ADD(IMAGE_ELEMENT_SIZE, "element size");
    ADD(IMAGE_ROW_PITCH, "row pitch");
    ADD(IMAGE_SLICE_PITCH, "slice pitch");
    ADD(IMAGE_WIDTH, "width");
    ADD(IMAGE_HEIGHT, "height");
    ADD(IMAGE_DEPTH, "depth");
    ADD(IMAGE_ARRAY_SIZE, "array size");
//  ADD(IMAGE_BUFFER, "buffer"); DEPRECATED
    ADD(IMAGE_NUM_MIP_LEVELS, "num mip levels");
    ADD(IMAGE_NUM_SAMPLES, "num samples");
    
    domain = DOMAIN_PIPE_INFO; /* cl_pipe_info */
    ADD(PIPE_PACKET_SIZE, "packet size");
    ADD(PIPE_MAX_PACKETS, "max packets");

    domain = DOMAIN_ADDRESSING_MODE; /* cl_addressing_mode */
    ADD(ADDRESS_NONE, "none");
    ADD(ADDRESS_CLAMP_TO_EDGE, "clamp to edge");
    ADD(ADDRESS_CLAMP, "clamp");
    ADD(ADDRESS_REPEAT, "repeat");
    ADD(ADDRESS_MIRRORED_REPEAT, "mirrored repeat");

    domain = DOMAIN_FILTER_MODE; /* cl_filter_mode */
    ADD(FILTER_NEAREST, "nearest");
    ADD(FILTER_LINEAR, "linear");

    domain = DOMAIN_SAMPLER_INFO; /* cl_sampler_info */
    ADD(SAMPLER_REFERENCE_COUNT, "reference count");
    ADD(SAMPLER_CONTEXT, "context");
    ADD(SAMPLER_NORMALIZED_COORDS, "normalized coords");
    ADD(SAMPLER_ADDRESSING_MODE, "addressing mode");
    ADD(SAMPLER_FILTER_MODE, "filter mode");
    ADD(SAMPLER_MIP_FILTER_MODE, "mip filter mode");
    ADD(SAMPLER_LOD_MIN, "lod min");
    ADD(SAMPLER_LOD_MAX, "lod max");

    domain = DOMAIN_PROGRAM_INFO; /* cl_program_info */
    ADD(PROGRAM_REFERENCE_COUNT, "reference count");
    ADD(PROGRAM_CONTEXT, "context");
    ADD(PROGRAM_NUM_DEVICES, "num devices");
    ADD(PROGRAM_DEVICES, "devices");
    ADD(PROGRAM_SOURCE, "source");
    ADD(PROGRAM_BINARY_SIZES, "binary sizes");
    ADD(PROGRAM_BINARIES, "binaries");
    ADD(PROGRAM_NUM_KERNELS, "num kernels");
    ADD(PROGRAM_KERNEL_NAMES, "kernel names");
    ADD(PROGRAM_IL, "il"); //CL_VERSION_2_1
    ADD(PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT, "scope global ctors present"); //CL_VERSION_2_2
    ADD(PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT, "scope global dtors present"); //CL_VERSION_2_2

    domain = DOMAIN_PROGRAM_BUILD_INFO; /* cl_program_build_info */
    ADD(PROGRAM_BUILD_STATUS, "status");
    ADD(PROGRAM_BUILD_OPTIONS, "options");
    ADD(PROGRAM_BUILD_LOG, "log");
    ADD(PROGRAM_BINARY_TYPE, "binary type");
    ADD(PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE, "global variable total size");
    
    domain = DOMAIN_PROGRAM_BINARY_TYPE; /* cl_program_binary_type */
    ADD(PROGRAM_BINARY_TYPE_NONE, "none");
    ADD(PROGRAM_BINARY_TYPE_COMPILED_OBJECT, "compiled object");
    ADD(PROGRAM_BINARY_TYPE_LIBRARY, "library");
    ADD(PROGRAM_BINARY_TYPE_EXECUTABLE, "executable");
    ADD(PROGRAM_BINARY_TYPE_INTERMEDIATE, "intermediate");

    domain = DOMAIN_KERNEL_INFO; /* cl_kernel_info */
    ADD(KERNEL_FUNCTION_NAME, "function name");
    ADD(KERNEL_NUM_ARGS, "num args");
    ADD(KERNEL_REFERENCE_COUNT, "reference count");
    ADD(KERNEL_CONTEXT, "context");
    ADD(KERNEL_PROGRAM, "program");
    ADD(KERNEL_ATTRIBUTES, "attributes");
    ADD(KERNEL_MAX_NUM_SUB_GROUPS, "max num sub groups"); //CL_VERSION_2_1
    ADD(KERNEL_COMPILE_NUM_SUB_GROUPS, "compile num sub groups");//CL_VERSION_2_1

    domain = DOMAIN_KERNEL_ARG_INFO; /* cl_kernel_arg_info */
    ADD(KERNEL_ARG_ADDRESS_QUALIFIER, "address qualifier");
    ADD(KERNEL_ARG_ACCESS_QUALIFIER, "access qualifier");
    ADD(KERNEL_ARG_TYPE_NAME, "type name");
    ADD(KERNEL_ARG_TYPE_QUALIFIER, "type qualifier");
    ADD(KERNEL_ARG_NAME, "name");

    domain = DOMAIN_KERNEL_ARG_ADDRESS_QUALIFIER; /* cl_kernel_arg_address_qualifier */
    ADD(KERNEL_ARG_ADDRESS_GLOBAL, "global");
    ADD(KERNEL_ARG_ADDRESS_LOCAL, "local");
    ADD(KERNEL_ARG_ADDRESS_CONSTANT, "constant");
    ADD(KERNEL_ARG_ADDRESS_PRIVATE, "private");

    domain = DOMAIN_KERNEL_ARG_ACCESS_QUALIFIER; /* cl_kernel_arg_access_qualifier */
    ADD(KERNEL_ARG_ACCESS_READ_ONLY, "read only");
    ADD(KERNEL_ARG_ACCESS_WRITE_ONLY, "write only");
    ADD(KERNEL_ARG_ACCESS_READ_WRITE, "read write");
    ADD(KERNEL_ARG_ACCESS_NONE, "none");
    
    domain = DOMAIN_KERNEL_WORK_GROUP_INFO; /* cl_kernel_work_group_info */
    ADD(KERNEL_WORK_GROUP_SIZE, "work group size");
    ADD(KERNEL_COMPILE_WORK_GROUP_SIZE, "compile work group size");
    ADD(KERNEL_LOCAL_MEM_SIZE, "local mem size");
    ADD(KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, "preferred work group size multiple");
    ADD(KERNEL_PRIVATE_MEM_SIZE, "private mem size");
    ADD(KERNEL_GLOBAL_WORK_SIZE, "global work size");

    domain = DOMAIN_KERNEL_SUB_GROUP_INFO; /* cl_kernel_sub_group_info */
    ADD(KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, "max sub group size for ndrange"); //CL_VERSION_2_1
    ADD(KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE, "sub group count for ndrange"); //CL_VERSION_2_1
    ADD(KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT, "local size for sub group count"); //CL_VERSION_2_1
    ADD(KERNEL_MAX_NUM_SUB_GROUPS, "max num sub groups"); //CL_VERSION_2_1
    ADD(KERNEL_COMPILE_NUM_SUB_GROUPS, "compile num sub groups"); //CL_VERSION_2_1
    
    domain = DOMAIN_KERNEL_EXEC_INFO; /* cl_kernel_exec_info */
    ADD(KERNEL_EXEC_INFO_SVM_PTRS, "svm ptrs");
    ADD(KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM, "svm fine grain system");

    domain = DOMAIN_EVENT_INFO; /* cl_event_info  */
    ADD(EVENT_COMMAND_QUEUE, "command queue");
    ADD(EVENT_COMMAND_TYPE, "command type");
    ADD(EVENT_REFERENCE_COUNT, "reference count");
    ADD(EVENT_COMMAND_EXECUTION_STATUS, "command execution status");
    ADD(EVENT_CONTEXT, "context");

    domain = DOMAIN_COMMAND_TYPE; /* cl_command_type */
    ADD(COMMAND_NDRANGE_KERNEL, "ndrange kernel");
    ADD(COMMAND_TASK, "task");
    ADD(COMMAND_NATIVE_KERNEL, "native kernel");
    ADD(COMMAND_READ_BUFFER, "read buffer");
    ADD(COMMAND_WRITE_BUFFER, "write buffer");
    ADD(COMMAND_COPY_BUFFER, "copy buffer");
    ADD(COMMAND_READ_IMAGE, "read image");
    ADD(COMMAND_WRITE_IMAGE, "write image");
    ADD(COMMAND_COPY_IMAGE, "copy image");
    ADD(COMMAND_COPY_IMAGE_TO_BUFFER, "copy image to buffer");
    ADD(COMMAND_COPY_BUFFER_TO_IMAGE, "copy buffer to image");
    ADD(COMMAND_MAP_BUFFER, "map buffer");
    ADD(COMMAND_MAP_IMAGE, "map image");
    ADD(COMMAND_UNMAP_MEM_OBJECT, "unmap mem object");
    ADD(COMMAND_MARKER, "marker");
    ADD(COMMAND_ACQUIRE_GL_OBJECTS, "acquire gl objects");
    ADD(COMMAND_RELEASE_GL_OBJECTS, "release gl objects");
    ADD(COMMAND_READ_BUFFER_RECT, "read buffer rect");
    ADD(COMMAND_WRITE_BUFFER_RECT, "write buffer rect");
    ADD(COMMAND_COPY_BUFFER_RECT, "copy buffer rect");
    ADD(COMMAND_USER, "user");
    ADD(COMMAND_BARRIER, "barrier");
    ADD(COMMAND_MIGRATE_MEM_OBJECTS, "migrate mem objects");
    ADD(COMMAND_FILL_BUFFER, "fill buffer");
    ADD(COMMAND_FILL_IMAGE, "fill image");
    ADD(COMMAND_SVM_FREE, "svm free");
    ADD(COMMAND_SVM_MEMCPY, "svm memcpy");
    ADD(COMMAND_SVM_MEMFILL, "svm memfill");
    ADD(COMMAND_SVM_MAP, "svm map");
    ADD(COMMAND_SVM_UNMAP, "svm unmap");
    ADD(COMMAND_GL_FENCE_SYNC_OBJECT_KHR, "gl fence sync object");

    domain = DOMAIN_COMMAND_EXECUTION_STATUS; /* command execution status */
    /* Not really an enum, may be also negative error code */
    ADD(COMPLETE, "complete");
    ADD(RUNNING, "running");
    ADD(SUBMITTED, "submitted");
    ADD(QUEUED, "queued");

    domain = DOMAIN_BUFFER_CREATE_TYPE; /* cl_buffer_create_type  */
    ADD(BUFFER_CREATE_TYPE_REGION, "region");

    domain = DOMAIN_PROFILING_INFO; /* cl_profiling_info  */
    ADD(PROFILING_COMMAND_QUEUED, "command queued");
    ADD(PROFILING_COMMAND_SUBMIT, "command submit");
    ADD(PROFILING_COMMAND_START, "command start");
    ADD(PROFILING_COMMAND_END, "command end");
    ADD(PROFILING_COMMAND_COMPLETE, "command complete");

    domain = DOMAIN_BUILD_STATUS; /* cl_build_status (cl_int) */
    ADD(BUILD_SUCCESS, "success");
    ADD(BUILD_NONE, "none");
    ADD(BUILD_ERROR, "error");
    ADD(BUILD_IN_PROGRESS, "in progress");

    domain = DOMAIN_GL_TEXTURE_INFO; /* cl_gl_texture_info */
    ADD(GL_TEXTURE_TARGET, "texture target");
    ADD(GL_MIPMAP_LEVEL, "mipmap level");
    ADD(GL_NUM_SAMPLES, "num samples");
 
    domain = DOMAIN_GL_CONTEXT_INFO; /* cl_gl_context_info */
    ADD(CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, "current device");
    ADD(DEVICES_FOR_GL_CONTEXT_KHR, "devices");

    domain = DOMAIN_QUEUE_PRIORITY; /* cl_queue_priority_khr */
    ADD(QUEUE_PRIORITY_HIGH_KHR, "high");
    ADD(QUEUE_PRIORITY_MED_KHR, "med");
    ADD(QUEUE_PRIORITY_LOW_KHR, "low");

    domain = DOMAIN_QUEUE_THROTTLE; /* cl_queue_throttle_khr */
    ADD(QUEUE_THROTTLE_HIGH_KHR, "high");
    ADD(QUEUE_THROTTLE_MED_KHR, "med");
    ADD(QUEUE_THROTTLE_LOW_KHR, "low");
#undef ADD

#define ADD(what, s) do { enums_new(L, domain, what, s); } while(0)
    domain = DOMAIN_GL_TEXTURE_TARGET; /* GL_TEXTURE_XXX */
    ADD(GL_TEXTURE_1D, "1d");
    ADD(GL_TEXTURE_2D, "2d");
    ADD(GL_TEXTURE_3D, "3d");
    ADD(GL_TEXTURE_1D_ARRAY, "1d array");
    ADD(GL_TEXTURE_2D_ARRAY, "2d array");
    ADD(GL_TEXTURE_BUFFER, "buffer");
    ADD(GL_TEXTURE_CUBE_MAP, "cube map");
    ADD(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "cube map positive x");
    ADD(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "cube map negative x");
    ADD(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "cube map positive y");
    ADD(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "cube map negative y");
    ADD(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "cube map positive z");
    ADD(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "cube map negative z");
    ADD(GL_TEXTURE_RECTANGLE, "rectangle");
    ADD(GL_TEXTURE_2D_MULTISAMPLE, "2d multisample");
    ADD(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, "2d multisample array");

    domain = DOMAIN_GL_OBJECT_TYPE; /* GL_OBJECT_TYPE */
    ADD(CL_GL_OBJECT_BUFFER, "buffer");
    ADD(CL_GL_OBJECT_TEXTURE2D, "texture 2d");
    ADD(CL_GL_OBJECT_TEXTURE3D, "texture 3d");
    ADD(CL_GL_OBJECT_RENDERBUFFER, "renderbuffer");
    ADD(CL_GL_OBJECT_TEXTURE2D_ARRAY, "texture 2d array");
    ADD(CL_GL_OBJECT_TEXTURE1D, "texture 1d");
    ADD(CL_GL_OBJECT_TEXTURE1D_ARRAY, "texture 1d array");
    ADD(CL_GL_OBJECT_TEXTURE_BUFFER, "texture buffer");
#undef ADD

    }

