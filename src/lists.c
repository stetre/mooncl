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
 | String List                                                                  |
 *------------------------------------------------------------------------------*/

char** checkstringlist(lua_State *L, int arg, cl_uint *count, int *err)
    {
    int t;
    char** list;
    const char* s;
    cl_uint i;
    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (char**)MallocNoErr(L, sizeof(char*) * (*count + 1));
    if(!list)
        { *err = ERR_MEMORY; return NULL; }
    for(i=0; i<*count; i++)
        {
        t = lua_rawgeti(L, arg, i+1);
        if(t != LUA_TSTRING)
            {
            lua_pop(L, 1);
            freestringlist(L, list, *count);
            *count = 0;
            *err = ERR_TYPE;
            return NULL;
            }
        s = lua_tostring(L, -1);
        list[i] = Strdup(L, s);
        lua_pop(L, 1);
        }
    /* list[*count]=NULL; */
    return list;
    }

void freestringlist(lua_State *L, char** list, cl_uint count)
    {
    cl_uint i;
    if(!list)
        return;
    for(i=0; i<count; i++)
        Free(L, list[i]);
    Free(L, list);
    }

void pushstringlist(lua_State *L, char** list, cl_uint count)
    {
    cl_uint i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushstring(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }

/*------------------------------------------------------------------------------*
 | Binary String List                                                           |
 *------------------------------------------------------------------------------*/


char** checklstringlist(lua_State *L, int arg, cl_uint *count, size_t **lengths_, int *err)
    {
#define lengths (*lengths_)
    int t;
    char** list;
    const char* s;
    cl_uint i;
    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }
    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }
    list = (char**)MallocNoErr(L, sizeof(char*) * (*count + 1));
    if(!list)
        { *err = ERR_MEMORY; return NULL; }
    lengths = (size_t*)MallocNoErr(L, sizeof(size_t) * (*count + 1));
    if(!lengths)
        { Free(L, list); *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        t = lua_rawgeti(L, arg, i+1);
        if(t != LUA_TSTRING)
            {
            lua_pop(L, 1);
            freestringlist(L, list, *count);
            *count = 0;
            *err = ERR_TYPE;
            return NULL;
            }
        s = lua_tolstring(L, -1, &(lengths[i]));
        list[i] = (char*)MallocNoErr(L, lengths[i]);
        if(!list[i])
            {
            lua_pop(L, 1);
            freestringlist(L, list, *count);
            *count = 0;
            *err = ERR_MEMORY;
            return NULL;
            }
        memcpy(list[i], s, lengths[i]);
        lua_pop(L, 1);
        }
    /* list[*count]=NULL; */
    return list;
#undef lengths
    }

void freelstringlist(lua_State *L, char** list, size_t *lengths, cl_uint count)
    {
    cl_uint i;
    if(!list)
        return;
    for(i=0; i<count; i++)
        Free(L, list[i]);
    Free(L, list);
    Free(L, lengths);
    }

void pushlstringlist(lua_State *L, char** list, size_t *lengths, cl_uint count)
    {
    cl_uint i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushlstring(L, list[i], lengths[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | size_t list                                                                  |
 *------------------------------------------------------------------------------*/

size_t* checksizelist(lua_State *L, int arg, cl_uint *count, int *err)
/* freesizelist() --> Free() */
    {
    size_t* list;
    cl_uint i;
    int isnum;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (size_t*)MallocNoErr(L, sizeof(size_t) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        list[i] = lua_tointegerx(L, -1, &isnum);
        lua_pop(L, 1);
        if(!isnum)
            { Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        }
    return list;
    }

void pushsizelist(lua_State *L, size_t *list, cl_uint count)
    {
    cl_uint i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }


/*------------------------------------------------------------------------------*
 | size_t[3] list                                                               |
 *------------------------------------------------------------------------------*/

int checksize3(lua_State *L, int arg, size_t dst[3])
    {
    int i, count, isnum;

    if(lua_isnoneornil(L, arg)) return ERR_NOTPRESENT;
    if(lua_type(L, arg) != LUA_TTABLE) return ERR_TABLE;

    count = luaL_len(L, arg);
    if(count != 3) return ERR_LENGTH;

    for(i=0; i<3; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(lua_isnoneornil(L, -1))
            dst[i] = 0;
        else
            {
            dst[i] = lua_tointegerx(L, -1, &isnum);
            if(!isnum)
                { lua_pop(L, 1); return ERR_TYPE; }
            }
        lua_pop(L, 1);
        }
    return 0;
    }

void pushsize3(lua_State *L, size_t list[3])
    {
    int i;
    lua_newtable(L);
    for(i=0; i<3; i++)
        {
        lua_pushinteger(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }

/*------------------------------------------------------------------------------*
 | lightuserdata (void*) list                                                   |
 *------------------------------------------------------------------------------*/

void** checklightuserdatalist(lua_State *L, int arg, cl_uint *count, int *err)
/* freelightuserdatalist() --> Free() */
    {
    void** list;
    cl_uint i;

    *count = 0;
    *err = 0;
    if(lua_isnoneornil(L, arg))
        { *err = ERR_NOTPRESENT; return NULL; }
    if(lua_type(L, arg) != LUA_TTABLE)
        { *err = ERR_TABLE; return NULL; }

    *count = luaL_len(L, arg);
    if(*count == 0)
        { *err = ERR_EMPTY; return NULL; }

    list = (void**)MallocNoErr(L, sizeof(void*) * (*count));
    if(!list)
        { *count = 0; *err = ERR_MEMORY; return NULL; }

    for(i=0; i<*count; i++)
        {
        lua_rawgeti(L, arg, i+1);
        if(lua_type(L, -1) != LUA_TLIGHTUSERDATA)
            { lua_pop(L, 1); Free(L, list); *count = 0; *err = ERR_TYPE; return NULL; }
        list[i] = lua_touserdata(L, -1);
        lua_pop(L, 1);
        }
    return list;
    }

void pushlightuserdatalist(lua_State *L, void* *list, cl_uint count)
    {
    cl_uint i;
    lua_newtable(L);
    for(i=0; i<count; i++)
        {
        lua_pushlightuserdata(L, list[i]);
        lua_rawseti(L, -2, i+1);
        }
    }





