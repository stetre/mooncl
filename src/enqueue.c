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

/* 
 * Common parameters:
 * we, wc = wait events list, count
 * ge = generate event (boolean)
 */

/*--------------------------------------------------------------------------*
 | Flush and Finish                                                         |
 *--------------------------------------------------------------------------*/

static int Flush(lua_State *L)
    {
    cl_queue queue = checkqueue(L, 1, NULL);
    cl_int ec = cl.Flush(queue);
    CheckError(L, ec);
    return 0;
    }

static int Finish(lua_State *L)
    {
    cl_queue queue = checkqueue(L, 1, NULL);
    cl_int ec = cl.Finish(queue);
    CheckError(L, ec);
    return 0;
    }

/*--------------------------------------------------------------------------*
 | Buffer                                                                   |
 *--------------------------------------------------------------------------*/

static int EnqueueReadBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);
    size_t offset = luaL_checkinteger(L, 4);
    size_t size = luaL_checkinteger(L, 5);
    void *ptr = checklightuserdata(L, 6);

//    checkbufferboundaries(L, buffer, offset, size);

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueReadBuffer(queue, buffer, blocking, offset, size, ptr, wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }

static int EnqueueWriteBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);
    size_t offset = luaL_checkinteger(L, 4);
    size_t size = luaL_checkinteger(L, 5);
    const void *ptr = checklightuserdata(L, 6);

//    checkbufferboundaries(L, buffer, offset, size);

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueWriteBuffer(queue, buffer, blocking, offset, size, ptr, wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueCopyBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer src_buffer = checkbuffer(L, 2, NULL);
    cl_buffer dst_buffer = checkbuffer(L, 3, NULL);
    size_t src_offset = luaL_checkinteger(L, 4);
    size_t dst_offset = luaL_checkinteger(L, 5);
    size_t size = luaL_checkinteger(L, 6);

    checkbufferboundaries(L, src_buffer, src_offset, size);
    checkbufferboundaries(L, dst_buffer, dst_offset, size);

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueCopyBuffer(queue, src_buffer, dst_buffer, src_offset, dst_offset, size,
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }

static int EnqueueFillBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t pattern_size;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    const void *pattern = luaL_checklstring(L, 3, &pattern_size);
    size_t offset = luaL_checkinteger(L, 4);
    size_t size = luaL_checkinteger(L, 5);

    checkbufferboundaries(L, buffer, offset, size);

    ge = optboolean(L, 7, 0);
    we = checkeventlist(L, 6, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 6, errstring(err));
    
    ec = cl.EnqueueFillBuffer(queue, buffer, pattern, pattern_size, offset, size, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


/*--------------------------------------------------------------------------*
 | Buffer Rect                                                              |
 *--------------------------------------------------------------------------*/

static int EnqueueReadBufferRect(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t buffer_origin[3], host_origin[3], region[3];
    size_t buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch;
    void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);

    err = checksize3(L, 4, buffer_origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, host_origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));
    buffer_row_pitch = luaL_checkinteger(L, 7);
    buffer_slice_pitch = luaL_checkinteger(L, 8);
    host_row_pitch = luaL_checkinteger(L, 9);
    host_slice_pitch = luaL_checkinteger(L, 10);

    ptr = checklightuserdata(L, 11);

    ge = optboolean(L, 13, 0);
    we = checkeventlist(L, 12, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 12, errstring(err));
    
    ec = cl.EnqueueReadBufferRect(queue, buffer, blocking, buffer_origin, host_origin, region, 
            buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueWriteBufferRect(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t buffer_origin[3], host_origin[3], region[3];
    size_t buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch;
    const void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);

    err = checksize3(L, 4, buffer_origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, host_origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));
    buffer_row_pitch = luaL_checkinteger(L, 7);
    buffer_slice_pitch = luaL_checkinteger(L, 8);
    host_row_pitch = luaL_checkinteger(L, 9);
    host_slice_pitch = luaL_checkinteger(L, 10);
    ptr = checklightuserdata(L, 11);

    ge = optboolean(L, 13, 0);
    we = checkeventlist(L, 12, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 12, errstring(err));
    
    ec = cl.EnqueueWriteBufferRect(queue, buffer, blocking, buffer_origin, host_origin, region, 
            buffer_row_pitch, buffer_slice_pitch, host_row_pitch, host_slice_pitch, ptr, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueCopyBufferRect(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t src_origin[3], dst_origin[3], region[3];
    size_t src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer src_buffer = checkbuffer(L, 2, NULL);
    cl_buffer dst_buffer = checkbuffer(L, 3, NULL);

    err = checksize3(L, 4, src_origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, dst_origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));
    src_row_pitch = luaL_checkinteger(L, 7);
    src_slice_pitch = luaL_checkinteger(L, 8);
    dst_row_pitch = luaL_checkinteger(L, 9);
    dst_slice_pitch = luaL_checkinteger(L, 10);

    ge = optboolean(L, 12, 0);
    we = checkeventlist(L, 11, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 11, errstring(err));
    
    ec = cl.EnqueueCopyBufferRect(queue, src_buffer, dst_buffer, src_origin, dst_origin, region, 
                src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }

/*--------------------------------------------------------------------------*
 | Image                                                                    |
 *--------------------------------------------------------------------------*/

static int EnqueueReadImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t origin[3], region[3];
    size_t row_pitch, slice_pitch;
    void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image image = checkimage(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);

    err = checksize3(L, 4, origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, region);
    if(err) return luaL_argerror(L, 5, errstring(err));
    row_pitch = luaL_checkinteger(L, 6);
    slice_pitch = luaL_checkinteger(L, 7);

    ptr = checklightuserdata(L, 8);

    ge = optboolean(L, 10, 0);
    we = checkeventlist(L, 9, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 9, errstring(err));
    
    ec = cl.EnqueueReadImage(queue, image, blocking, origin, region, row_pitch, slice_pitch, ptr, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueWriteImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t origin[3], region[3];
    size_t input_row_pitch, input_slice_pitch;
    const void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image image = checkimage(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);

    err = checksize3(L, 4, origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, region);
    if(err) return luaL_argerror(L, 5, errstring(err));
    input_row_pitch = luaL_checkinteger(L, 6);
    input_slice_pitch = luaL_checkinteger(L, 7);

    ptr = checklightuserdata(L, 8);

    ge = optboolean(L, 10, 0);
    we = checkeventlist(L, 9, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 9, errstring(err));
    
    ec = cl.EnqueueWriteImage(queue, image, blocking, origin, region, 
            input_row_pitch, input_slice_pitch, ptr, wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }



static int EnqueueFillImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t origin[3], region[3];
    size_t fill_color_size;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image image = checkimage(L, 2, NULL);
    const void *fill_color = luaL_checklstring(L, 3, &fill_color_size);
    // fill_color may be either: float, float[4], int[4], uint[4]
    //  e.g. fill_color = cl.pack('float', { 0.1, 0.5, 0.0, 1.0 })
    //  e.g. fill_color = cl.pack('float', 0.1)

    err = checksize3(L, 4, origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, region);
    if(err) return luaL_argerror(L, 5, errstring(err));

    ge = optboolean(L, 7, 0);
    we = checkeventlist(L, 6, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 6, errstring(err));
    
    ec = cl.EnqueueFillImage(queue, image, fill_color, origin, region, wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueCopyImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t src_origin[3], dst_origin[3], region[3];
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image src_image = checkimage(L, 2, NULL);
    cl_image dst_image = checkimage(L, 3, NULL);

    err = checksize3(L, 4, src_origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, dst_origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueCopyImage(queue, src_image, dst_image, src_origin, dst_origin, region, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueCopyImageToBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t src_origin[3], region[3];
    size_t dst_offset;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image src_image = checkimage(L, 2, NULL);
    cl_buffer dst_buffer = checkbuffer(L, 3, NULL);

    err = checksize3(L, 4, src_origin);
    if(err) return luaL_argerror(L, 4, errstring(err));
    err = checksize3(L, 5, region);
    if(err) return luaL_argerror(L, 5, errstring(err));
    dst_offset = luaL_checkinteger(L, 6);

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueCopyImageToBuffer(queue, src_image, dst_buffer, src_origin, region, dst_offset, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueCopyBufferToImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t dst_origin[3], region[3];
    size_t src_offset;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer src_buffer = checkbuffer(L, 2, NULL);
    cl_image dst_image = checkimage(L, 3, NULL);

    src_offset = luaL_checkinteger(L, 4);
    err = checksize3(L, 5, dst_origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ec = cl.EnqueueCopyBufferToImage(queue, src_buffer, dst_image, src_offset, dst_origin, region, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }

/*--------------------------------------------------------------------------*
 | Map                                                                      |
 *--------------------------------------------------------------------------*/

static int EnqueueMapBuffer(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_buffer buffer = checkbuffer(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);
    cl_map_flags flags = checkflags(L, 4);
    size_t offset = luaL_checkinteger(L, 5);
    size_t size = luaL_checkinteger(L, 6);

    checkbufferboundaries(L, buffer, offset, size);

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ptr = cl.EnqueueMapBuffer(queue, buffer, blocking, flags, offset, size,
                wc, we, ge ? &event : NULL, &ec);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }

    lua_pushlightuserdata(L, ptr);
    if(event)
        { 
        newevent(L, ud->context, event);
        return 2;
        }
    return 1;
    }

static int EnqueueMapImage(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t origin[3], region[3];
    size_t image_row_pitch = 0, image_slice_pitch = 0;
    void *ptr;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_image image = checkimage(L, 2, NULL);
    cl_bool blocking = checkboolean(L, 3);
    cl_map_flags flags = checkflags(L, 4);
    err = checksize3(L, 5, origin);
    if(err) return luaL_argerror(L, 5, errstring(err));
    err = checksize3(L, 6, region);
    if(err) return luaL_argerror(L, 6, errstring(err));

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 7, errstring(err));
    
    ptr = cl.EnqueueMapImage(queue, image, blocking, flags, origin, region, 
                &image_row_pitch, &image_slice_pitch, wc, we, ge ? &event : NULL, &ec);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }

    lua_pushlightuserdata(L, ptr);
    lua_pushinteger(L, image_row_pitch);
    lua_pushinteger(L, image_slice_pitch);
    if(event)
        { 
        newevent(L, ud->context, event);
        return 4;
        }
    return 3;
    }

static int EnqueueUnmapMemObject(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_mem mem = checkmemobject(L, 2, NULL);
    void *ptr = checklightuserdata(L, 3);
    
    ge = optboolean(L, 6, 0);
    we = checkeventlist(L, 5, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 5, errstring(err));

    ec = cl.EnqueueUnmapMemObject(queue, mem, ptr, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueSVMMap(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_bool blocking = checkboolean(L, 2);
    cl_map_flags flags = checkflags(L, 3);
    void *ptr = checklightuserdata(L, 4);
    size_t size = luaL_checkinteger(L, 5);

    CheckPfn_2_0(L, EnqueueSVMMap);

    ge = optboolean(L, 7, 0);
    we = checkeventlist(L, 6, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 6, errstring(err));
    
    ec = cl.EnqueueSVMMap(queue, blocking, flags, ptr, size, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }

    if(event)
        { 
        newevent(L, ud->context, event);
        return 1;
        }
    return 0;
    }


static int EnqueueSVMUnmap(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    void *ptr = checklightuserdata(L, 2);
    
    CheckPfn_2_0(L, EnqueueSVMUnmap);

    ge = optboolean(L, 5, 0);
    we = checkeventlist(L, 4, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 4, errstring(err));

    ec = cl.EnqueueSVMUnmap(queue, ptr, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }



/*--------------------------------------------------------------------------*
 | Shared Virtual Memory                                                    |
 *--------------------------------------------------------------------------*/

static int EnqueueSVMFree(lua_State *L)
/* event = cl.enqueue_svm_free(queue, {svm}, we, ge) */
    {
    int err, ge;
    ud_t *ud, *svm_ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count,  i;
    cl_event *we = NULL;
    void ** ptrs = NULL;
    cl_svm *svms;
    cl_queue queue = checkqueue(L, 1, &ud);

    CheckPfn_2_0(L, EnqueueSVMFree);

    svms = checksvmlist(L, 2, &count, &err);
    if(err) return luaL_argerror(L, 2, errstring(err));

#define CLEANUP() do {  \
    Free(L, svms);      \
    Free(L, ptrs);      \
} while(0)

    ptrs = (void**)MallocNoErr(L, count * sizeof(void*));
    if(!ptrs) { CLEANUP(); return luaL_error(L, errstring(ERR_MEMORY)); }

    for(i=0; i<count; i++)
        {
        svm_ud = userdata(svms[i]);
        MarkSvmDontFree(svm_ud);  /* so the destructor won't call cl.SVMFree() */
        ptrs[i] = svms[i]->ptr;
        svm_ud->destructor(L, svm_ud);
        }
    
    ge = optboolean(L, 4, 0);
    we = checkeventlist(L, 3, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 3, errstring(err)); }

    ec = cl.EnqueueSVMFree(queue, count, ptrs, NULL, NULL, wc, we, ge ? &event : NULL);
    CLEANUP();
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }

static int EnqueueSVMMemcpy(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_bool blocking = checkboolean(L, 2);
    void *dst_ptr = checklightuserdata(L, 3);
    const void *src_ptr = checklightuserdata(L, 4);
    size_t size = luaL_checkinteger(L, 5);

    CheckPfn_2_0(L, EnqueueSVMMemcpy);

    ge = optboolean(L, 7, 0);
    we = checkeventlist(L, 6, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 6, errstring(err));
    
    ec = cl.EnqueueSVMMemcpy(queue, blocking, dst_ptr, src_ptr, size, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }

    if(event)
        { 
        newevent(L, ud->context, event);
        return 1;
        }
    return 0;
    }

static int EnqueueSVMMemFill(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    size_t pattern_size;
    cl_queue queue = checkqueue(L, 1, &ud);
    void *svm_ptr = checklightuserdata(L, 2);
    const void *pattern = luaL_checklstring(L, 3, &pattern_size);
    size_t size = luaL_checkinteger(L, 4);

    CheckPfn_2_0(L, EnqueueSVMMemFill);

    ge = optboolean(L, 6, 0);
    we = checkeventlist(L, 5, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 5, errstring(err));
    
    ec = cl.EnqueueSVMMemFill(queue, svm_ptr, pattern, pattern_size, size, 
                wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }

    if(event)
        { 
        newevent(L, ud->context, event);
        return 1;
        }
    return 0;
    }

/*--------------------------------------------------------------------------*
 | Migrate                                                                  |
 *--------------------------------------------------------------------------*/

static int EnqueueMigrateMemObjects(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count;
    cl_event *we = NULL;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_mem_migration_flags flags = checkflags(L, 2);
    const cl_mem *mem_objects = checkmemobjectlist(L, 3, &count, &err);
    if(err)
        return luaL_argerror(L, 3, errstring(err));
#define CLEANUP() do {              \
    Free(L, (void*)mem_objects);    \
    Free(L, we);            \
} while(0)
    
    ge = optboolean(L, 5, 0);
    we = checkeventlist(L, 4, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 4, errstring(err)); }

    ec = cl.EnqueueMigrateMemObjects(queue, count, mem_objects, flags, wc, we, ge ? &event : NULL);
    CLEANUP();
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }


static int EnqueueSVMMigrateMem(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count, count1, i;
    size_t *offsets = NULL, *sizes = NULL;
    cl_event *we = NULL;
    void ** ptrs = NULL;
    cl_svm *svms;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_mem_migration_flags flags = checkflags(L, 2);

    CheckPfn_2_1(L, EnqueueSVMMigrateMem);

    svms = checksvmlist(L, 3, &count, &err);
    if(err) return luaL_argerror(L, 3, errstring(err));

#define CLEANUP() do {              \
    Free(L, svms);          \
    Free(L, offsets);               \
    Free(L, sizes);                 \
    Free(L, ptrs);                  \
} while(0)

    offsets = checksizelist(L, 4, &count1, &err);
    if(err)
        { CLEANUP(); return luaL_argerror(L, 4, errstring(err)); }
    if(count1 != count)
        { CLEANUP(); return luaL_argerror(L, 4, errstring(ERR_LENGTH)); }
        
    sizes = checksizelist(L, 5, &count1, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 5, errstring(err)); }
    if(count1 > 0 && count1 != count)
        { CLEANUP(); return luaL_argerror(L, 5, errstring(ERR_LENGTH)); }

    ptrs = (void**)MallocNoErr(L, count * sizeof(void*));
    if(!ptrs) { CLEANUP(); return luaL_error(L, errstring(ERR_MEMORY)); }
    for(i=0; i<count; i++)
        ptrs[i] = (char*)svms[i]->ptr + offsets[i];
    
    ge = optboolean(L, 7, 0);
    we = checkeventlist(L, 6, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 6, errstring(err)); }

    ec = cl.EnqueueSVMMigrateMem(queue, count, (const void**)ptrs, sizes, flags, wc, we, ge ? &event : NULL);
    CLEANUP();
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }


/*--------------------------------------------------------------------------*
 | Kernels and synch                                                        |
 *--------------------------------------------------------------------------*/

static int EnqueueNDRangeKernel(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count;
    size_t *global_work_offset = NULL;
    size_t *global_work_size = NULL;
    size_t *local_work_size = NULL;
    cl_event *we = NULL;
    cl_queue queue = checkqueue(L, 1, &ud);
    cl_kernel kernel = checkkernel(L, 2, NULL);
    cl_uint work_dim = luaL_checkinteger(L, 3);
    if(work_dim == 0)
        return luaL_argerror(L, 3, "work_dim must be positive");
#define CLEANUP() do {              \
    Free(L, global_work_offset);    \
    Free(L, global_work_size);      \
    Free(L, local_work_size);       \
    Free(L, we);            \
} while(0)
    
    global_work_offset = checksizelist(L, 4, &count, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 4, errstring(err)); }
    if(count > 0 && count != work_dim)
        { CLEANUP(); return luaL_argerror(L, 4, "table length must be work_dim"); }
        
    global_work_size = checksizelist(L, 5, &count, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 5, errstring(err)); }
    if(count > 0 && count != work_dim)
        { CLEANUP(); return luaL_argerror(L, 5, "table length must be work_dim"); }

    local_work_size = checksizelist(L, 6, &count, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 6, errstring(err)); }
    if(count > 0 && count != work_dim)
        { CLEANUP(); return luaL_argerror(L, 6, "table length must be work_dim"); }

    ge = optboolean(L, 8, 0);
    we = checkeventlist(L, 7, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 7, errstring(err)); }

    ec = cl.EnqueueNDRangeKernel(queue, kernel, work_dim, 
            global_work_offset, global_work_size, local_work_size, wc, we, ge ? &event : NULL);
    CLEANUP();
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }


static int EnqueueTask(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_uint work_dim = 1;
    size_t work_size = 1;
    cl_event *we = NULL;

    cl_queue queue = checkqueue(L, 1, &ud);
    cl_kernel kernel = checkkernel(L, 2, NULL);
    
    ge = optboolean(L, 4, 0);
    we = checkeventlist(L, 3, &wc, &err);
    if(err < 0)
        { return luaL_argerror(L, 3, errstring(err)); }

    ec = cl.EnqueueNDRangeKernel(queue, kernel, work_dim, NULL, &work_size, &work_size, 
                    wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }




#if 0
typedef void (*NATIVEKERNEL)(void*);
static void mynativekernel(void *args)
    {
    printf("mynativekernel %p\n", args);
    }

static NATIVEKERNEL Foo = mynativekernel;

static int GetFoo(lua_State *L)
    {
    lua_pushlightuserdata(L, &Foo);
    return 1;
    }

static int DoFoo(lua_State *L)
    {
    NATIVEKERNEL *zzz = (NATIVEKERNEL*)checklightuserdata(L, 1);
    (*zzz)((void*)0xaabbccdd);
    return 0;
    }
        { "get_foo", GetFoo },
        { "do_foo", DoFoo },


//cl_int cl.EnqueueNativeKernel(cl_queue queue, void (CL_CALLBACK *user_func)(void *), void *args, size_t cb_args, cl_uint num_mem_objects, const cl_mem *mem_list, const void **args_mem_loc, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
static int EnqueueNativeKernel(lua_State *L) //@@
    {
    ud_t *ud;
    cl_queue queue = checkqueue(L, 1, &ud);
    //NATIVEKERNEL *user_func = (NATIVEKERNEL*)checklightuserdata(L, 2);
    (void)queue;
    return 0;
    }
#endif

static int EnqueueMarkerWithWaitList(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    
    ge = optboolean(L, 3, 0);
    we = checkeventlist(L, 2, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 2, errstring(err));

    ec = cl.EnqueueMarkerWithWaitList(queue, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }


static int EnqueueBarrierWithWaitList(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc;
    cl_event *we;
    cl_queue queue = checkqueue(L, 1, &ud);
    
    ge = optboolean(L, 3, 0);
    we = checkeventlist(L, 2, &wc, &err);
    if(err < 0)
        return luaL_argerror(L, 2, errstring(err));

    ec = cl.EnqueueBarrierWithWaitList(queue, wc, we, ge ? &event : NULL);
    Free(L, we);
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
    }

static int EnqueueAcquireGLObjects(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count;
    cl_event *we = NULL;
    const cl_mem *mem_objects;
    cl_queue queue = checkqueue(L, 1, &ud);

    CheckExtPfn(L, ud, EnqueueAcquireGLObjects);
    
    mem_objects = checkmemobjectlist(L, 2, &count, &err);
    if(err)
        return luaL_argerror(L, 2, errstring(err));
#define CLEANUP() do {              \
    Free(L, (void*)mem_objects);    \
    Free(L, we);                    \
} while(0)
    
    ge = optboolean(L, 4, 0);
    we = checkeventlist(L, 3, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 3, errstring(err)); }

    ec = ud->clext->EnqueueAcquireGLObjects(queue, count, mem_objects, wc, we, ge ? &event : NULL);
    CLEANUP();
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }

static int EnqueueReleaseGLObjects(lua_State *L)
    {
    int err, ge;
    ud_t *ud;
    cl_int ec;
    cl_event event = 0;
    cl_uint wc, count;
    cl_event *we = NULL;
    const cl_mem *mem_objects;
    cl_queue queue = checkqueue(L, 1, &ud);

    CheckExtPfn(L, ud, EnqueueReleaseGLObjects);
    
    mem_objects = checkmemobjectlist(L, 2, &count, &err);
    if(err)
        return luaL_argerror(L, 2, errstring(err));
#define CLEANUP() do {              \
    Free(L, (void*)mem_objects);    \
    Free(L, we);                    \
} while(0)
    
    ge = optboolean(L, 4, 0);
    we = checkeventlist(L, 3, &wc, &err);
    if(err < 0)
        { CLEANUP(); return luaL_argerror(L, 3, errstring(err)); }

    ec = ud->clext->EnqueueReleaseGLObjects(queue, count, mem_objects, wc, we, ge ? &event : NULL);
    CLEANUP();
    if(ec)
        { CheckError(L, ec); return 0; }        
    if(event)
        return newevent(L, ud->context, event);
    return 0;
#undef CLEANUP
    }



static const struct luaL_Reg Functions[] = 
    {
        { "flush", Flush },
        { "finish", Finish },
        { "enqueue_read_buffer", EnqueueReadBuffer },
        { "enqueue_write_buffer", EnqueueWriteBuffer },
        { "enqueue_copy_buffer", EnqueueCopyBuffer },
        { "enqueue_fill_buffer", EnqueueFillBuffer },
        { "enqueue_read_buffer_rect", EnqueueReadBufferRect },
        { "enqueue_write_buffer_rect", EnqueueWriteBufferRect },
        { "enqueue_copy_buffer_rect", EnqueueCopyBufferRect },
        { "enqueue_read_image", EnqueueReadImage },
        { "enqueue_write_image", EnqueueWriteImage },
        { "enqueue_fill_image", EnqueueFillImage },
        { "enqueue_copy_image", EnqueueCopyImage },
        { "enqueue_copy_image_to_buffer", EnqueueCopyImageToBuffer },
        { "enqueue_copy_buffer_to_image", EnqueueCopyBufferToImage },
        { "enqueue_map_buffer", EnqueueMapBuffer },
        { "enqueue_unmap_buffer", EnqueueUnmapMemObject },
        { "enqueue_map_image", EnqueueMapImage },
        { "enqueue_unmap_image", EnqueueUnmapMemObject },
        { "enqueue_migrate_mem_objects", EnqueueMigrateMemObjects },
        { "enqueue_ndrange_kernel", EnqueueNDRangeKernel },
        { "enqueue_task", EnqueueTask },
//      { "enqueue_native_kernel", EnqueueNativeKernel },
        { "enqueue_marker", EnqueueMarkerWithWaitList },
        { "enqueue_barrier", EnqueueBarrierWithWaitList },
        { "enqueue_svm_free", EnqueueSVMFree },
        { "enqueue_svm_memcpy", EnqueueSVMMemcpy },
        { "enqueue_svm_mem_fill", EnqueueSVMMemFill },
        { "enqueue_svm_map", EnqueueSVMMap },
        { "enqueue_svm_unmap", EnqueueSVMUnmap },
        { "enqueue_svm_migrate_mem", EnqueueSVMMigrateMem },
        { "enqueue_acquire_gl_objects", EnqueueAcquireGLObjects },
        { "enqueue_release_gl_objects", EnqueueReleaseGLObjects },
        { NULL, NULL } /* sentinel */
    };


void mooncl_open_enqueue(lua_State *L)
    {
    luaL_setfuncs(L, Functions, 0);
    }

