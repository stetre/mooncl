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
 
/* @@TODO: what if host and device have different endianness?
 */

size_t sizeofprimtype(int type)
    {
    switch(type)
        {
        case NONCL_TYPE_CHAR:   return sizeof(cl_char);
        case NONCL_TYPE_UCHAR:  return sizeof(cl_uchar);
        case NONCL_TYPE_SHORT:  return sizeof(cl_short);
        case NONCL_TYPE_USHORT: return sizeof(cl_ushort);
        case NONCL_TYPE_INT:    return sizeof(cl_int);
        case NONCL_TYPE_UINT:   return sizeof(cl_uint);
        case NONCL_TYPE_LONG:   return sizeof(cl_long);
        case NONCL_TYPE_ULONG: return sizeof(cl_ulong);
        case NONCL_TYPE_HALF:   return sizeof(cl_half);
        case NONCL_TYPE_FLOAT:  return sizeof(cl_float);
        case NONCL_TYPE_DOUBLE: return sizeof(cl_double);
        default:
            return 0;
        }
    return 0;
    }

static int Sizeof(lua_State *L)
/* size = sizeof(type) */
    {
    int type = checkprimtype(L, 1);
    lua_pushinteger(L, sizeofprimtype(type));
    return 1;
    }

static int Flatten1_(lua_State *L, int table_index, int cur_index, int arg)
    {
    int len, i, top, m, n=0;

    if(lua_type(L, arg) != LUA_TTABLE)
        return luaL_error(L, "table expected");

    lua_len(L, arg);
    len = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if(len==0) return n;

    for(i=1; i<=len; i++)
        {
        lua_geti(L, arg, i);
        top = lua_gettop(L);
        if(lua_type(L, top) == LUA_TTABLE)
            {
            m = Flatten1_(L, table_index, cur_index, top);
            n += m;
            cur_index += m;
            lua_remove(L, top);
            }
        else
            {
            n++;
            cur_index++;
            lua_rawseti(L, table_index, cur_index);
            }
        }
    return n;
    }

int toflattable(lua_State *L, int arg)
/* Creates a flat table with all the arguments starting from arg, and leaves 
 * it on top of the stack.
 */
    {
    int table_index, last_arg, i, n;
    if(lua_type(L, arg) == LUA_TTABLE)
        {
        lua_newtable(L);
        n = Flatten1_(L, lua_gettop(L), 0, arg);
        }
    else
        {
        /* create a table with all the arguments, and flatten it */
        last_arg = lua_gettop(L);
        lua_newtable(L);
        table_index = lua_gettop(L);
        for(i=arg; i <= last_arg; i++)
            {
            lua_pushvalue(L, i);
            lua_rawseti(L, table_index, i-arg+1);
            }
        lua_newtable(L);
        n = Flatten1_(L, lua_gettop(L), 0, table_index);
        }
    return n;
    }

static int Flatten(lua_State *L)
    {
    int n, i, table_index;
    n = toflattable(L, 1);
    table_index = lua_gettop(L);
    luaL_checkstack(L, n, "too many elements, cannot grow Lua stack");
    for(i = 0; i < n; i++)
        lua_rawgeti(L, table_index, i+1);
    return n;
    }

static int FlattenTable(lua_State *L)
    {
    toflattable(L, 1);
    return 1;
    }

/*-----------------------------------------------------------------------------*/

#define PACK(T, what) /* what= number or integer */ \
static int Pack##T(lua_State *L, size_t n, void *dst, size_t dstsize, int *faulty_element)  \
    {                                       \
    int isnum;                              \
    size_t i;                               \
    T *data = (T*)dst;                      \
    if(faulty_element) *faulty_element = 0; \
    if(dstsize < (n * sizeof(T)))           \
        return ERR_LENGTH;                  \
    for(i = 0; i < n; i++)                  \
        {                                   \
        lua_rawgeti(L, -1, i+1);            \
        data[i] = lua_to##what##x(L, -1, &isnum); \
        if(!isnum)                          \
            {                               \
            if(faulty_element) *faulty_element = i+1;   \
            return ERR_TYPE; /* element i+1 is not a #what */ \
            }                               \
        lua_pop(L, 1);                      \
        }                                   \
    return 0;                               \
    }

#define PACK_NUMBERS(T)     PACK(T, number)
#define PACK_INTEGERS(T)    PACK(T, integer)

PACK_NUMBERS(cl_half)
PACK_NUMBERS(cl_float)
PACK_NUMBERS(cl_double)
PACK_INTEGERS(cl_char)
PACK_INTEGERS(cl_uchar)
PACK_INTEGERS(cl_short)
PACK_INTEGERS(cl_ushort)
PACK_INTEGERS(cl_int)
PACK_INTEGERS(cl_uint)
PACK_INTEGERS(cl_long)
PACK_INTEGERS(cl_ulong)

static int Pack(lua_State *L)
    {
    int err = 0;
    void *dst = NULL;
    size_t dstsize = 0;
    int type = checkprimtype(L, 1);
    size_t n = toflattable(L, 2);
    switch(type)
        {
#define P(T) do { dstsize = n * sizeof(T);      \
                  dst = Malloc(L, dstsize);     \
                  err = Pack##T(L, n, dst, dstsize, NULL); } while(0)

        case NONCL_TYPE_CHAR:   P(cl_char); break;
        case NONCL_TYPE_UCHAR:  P(cl_uchar); break;
        case NONCL_TYPE_SHORT:  P(cl_short); break;
        case NONCL_TYPE_USHORT: P(cl_ushort); break;
        case NONCL_TYPE_INT:    P(cl_int); break;
        case NONCL_TYPE_UINT:   P(cl_uint); break;
        case NONCL_TYPE_LONG:   P(cl_long); break;
        case NONCL_TYPE_ULONG:  P(cl_ulong); break;
        case NONCL_TYPE_HALF:   P(cl_half); break;
        case NONCL_TYPE_FLOAT:  P(cl_float); break;
        case NONCL_TYPE_DOUBLE: P(cl_double); break;
        default:
            return unexpected(L);
#undef P
        }
    if(err)
        {
        Free(L, dst);
        return luaL_argerror(L, 2, errstring(err));
        }
    lua_pushlstring(L, (char*)dst, dstsize);
    Free(L, dst);
    return 1;
    }

/*-----------------------------------------------------------------------------*/

#define UNPACK(T, what) /* what= number or integer */   \
static int Unpack##T(lua_State *L, const void* data, size_t len) \
    {                                                   \
    size_t n;                                           \
    size_t i=0;                                         \
    if((len < sizeof(T)) || (len % sizeof(T)) != 0)     \
        return ERR_LENGTH;                              \
    n = len / sizeof(T);                                \
    lua_newtable(L);                                    \
    for(i = 0; i < n; i++)                              \
        {                                               \
        lua_push##what(L, ((T*)data)[i]);               \
        lua_rawseti(L, -2, i+1);                        \
        }                                               \
    return 0;                                           \
    }

#define UNPACK_NUMBERS(T)   UNPACK(T, number)
#define UNPACK_INTEGERS(T)  UNPACK(T, integer)


UNPACK_NUMBERS(cl_half)
UNPACK_NUMBERS(cl_float)
UNPACK_NUMBERS(cl_double)
UNPACK_INTEGERS(cl_char)
UNPACK_INTEGERS(cl_uchar)
UNPACK_INTEGERS(cl_short)
UNPACK_INTEGERS(cl_ushort)
UNPACK_INTEGERS(cl_int)
UNPACK_INTEGERS(cl_uint)
UNPACK_INTEGERS(cl_long)
UNPACK_INTEGERS(cl_ulong)

static int Unpack_(lua_State *L, int type, const void *data, size_t len)
    {
    int err = 0;
    switch(type)
        {
        case NONCL_TYPE_CHAR:   err = Unpackcl_char(L, data, len); break;
        case NONCL_TYPE_UCHAR:  err = Unpackcl_uchar(L, data, len); break;
        case NONCL_TYPE_SHORT:  err = Unpackcl_short(L, data, len); break;
        case NONCL_TYPE_USHORT: err = Unpackcl_ushort(L, data, len); break;
        case NONCL_TYPE_INT:    err = Unpackcl_int(L, data, len); break;
        case NONCL_TYPE_UINT:   err = Unpackcl_uint(L, data, len); break;
        case NONCL_TYPE_LONG:   err = Unpackcl_long(L, data, len); break;
        case NONCL_TYPE_ULONG:  err = Unpackcl_ulong(L, data, len); break;
        case NONCL_TYPE_HALF:   err = Unpackcl_half(L, data, len); break;
        case NONCL_TYPE_FLOAT:  err = Unpackcl_float(L, data, len); break;
        case NONCL_TYPE_DOUBLE: err = Unpackcl_double(L, data, len); break;
        default:
            return unexpected(L);
        }
    if(err)
        return luaL_error(L, errstring(err));
    return 1;
    }


static int Unpack(lua_State *L)
    {
    size_t len;
    int type = checkprimtype(L, 1);
    const void *data = luaL_checklstring(L, 2, &len);
    return Unpack_(L, type, data, len);
    }

/*-----------------------------------------------------------------------------*/


int testdata(lua_State *L, int type, size_t n, void *dst, size_t dstsize)
/* expects, on top of the stack, a flat table containing n elements of the given type
 * dst must point to at least n * sizeofprimtype(type) bytes of memory, where
 * the data will be packed according to the given type
 */
    {
    int err = 0;
    switch(type)
        {
#define P(T) do { err = Pack##T(L, n, dst, dstsize, NULL); } while(0)

        case NONCL_TYPE_CHAR:   P(cl_char); break;
        case NONCL_TYPE_UCHAR:  P(cl_uchar); break;
        case NONCL_TYPE_SHORT:  P(cl_short); break;
        case NONCL_TYPE_USHORT: P(cl_ushort); break;
        case NONCL_TYPE_INT:    P(cl_int); break;
        case NONCL_TYPE_UINT:   P(cl_uint); break;
        case NONCL_TYPE_LONG:   P(cl_long); break;
        case NONCL_TYPE_ULONG:  P(cl_ulong); break;
        case NONCL_TYPE_HALF:   P(cl_half); break;
        case NONCL_TYPE_FLOAT:  P(cl_float); break;
        case NONCL_TYPE_DOUBLE: P(cl_double); break;
        default:
            return unexpected(L);
#undef P
        }
    return err;
    }


int checkdata(lua_State *L, int arg, int type, void *dst, size_t dstsize)
    {
    size_t n = toflattable(L, arg);
    int err = testdata(L, type, n, dst, dstsize);
    if(err)
        {
        lua_pop(L, 1); /* flat table */
        return luaL_argerror(L, arg, errstring(err));
        }
    return 0;
    }


int pushdata(lua_State *L, int type, void *data, size_t datalen)
    {
    return Unpack_(L, type, data, datalen);
    }


static const struct luaL_Reg Functions[] = 
    {
        { "flatten", Flatten },
        { "flatten_table", FlattenTable },
        { "sizeof", Sizeof },
        { "pack", Pack },
        { "unpack", Unpack },
        { NULL, NULL } /* sentinel */
    };

void mooncl_open_datahandling(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }


