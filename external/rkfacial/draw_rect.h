/*
 * Rockchip App
 *
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 * author: hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __DRAW_RECT_H__
#define __DRAW_RECT_H__

#include <time.h>
#include <stdio.h>

typedef enum {
    COLOR_Y,
    COLOR_R,
    COLOR_G,
    COLOR_B,
    COLOR_BK,
    COLOR_W,
} COLOR_Type;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} YUV_Rect;

typedef struct {
    int x;
    int y;
} YUV_Point;

typedef struct {
    int Y;
    int U;
    int V;
} YUV_Color;

YUV_Color set_yuv_color(COLOR_Type color_type);

void yuv420_draw_line(void* imgdata,
    int width,
    int height,
    YUV_Point startPoint,
    YUV_Point endPoint,
    YUV_Color color);

void yuv420_draw_rectangle(void* imgdata,
    int width,
    int height,
    YUV_Rect rect_rio,
    YUV_Color color);

#endif
