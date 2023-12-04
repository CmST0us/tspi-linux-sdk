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

#include "draw_rect.h"

#ifndef ABS
#define ABS(x)              (((x)<0) ? -(x) : (x))
#endif

static void YUV_SWAP(int *x, int *y)
{
    int tmp = 0;

    tmp = *x;
    *x = *y;
    *y = tmp;
}

YUV_Color set_yuv_color(COLOR_Type color_type)
{
    YUV_Color color;

    switch (color_type) {
        case COLOR_Y:
            color.Y = 226;
            color.U = 0;
            color.V = 149;
        break;

        case COLOR_R:
            color.Y = 76;
            color.U = 85;
            color.V = 255;
        break;

        case COLOR_G:
            color.Y = 150;
            color.U = 44;
            color.V = 21;
        break;

        case COLOR_B:
            color.Y = 29;
            color.U = 255;
            color.V = 107;
        break;

        case COLOR_BK:
            color.Y = 16;
            color.U = 128;
            color.V = 128;
        break;

        default:
            color.Y = 128;
            color.U = 128;
            color.V = 128;
        break;
    }

    return color;
}

static int yuv420_draw_point(unsigned char* imgdata,
    int width,
    int height,
    YUV_Point Point,
    YUV_Color color)
{
    int imgSize;
    int x, y;

    if (!imgdata)
        return -1;

    if (width < 0 || height < 0)
        return -1;

    if (Point.x < 0 || Point.x > width ||
        Point.y < 0 || Point.y > height)
        return -1;

    if (Point.x % 2)
        Point.x--;
    if (Point.y % 2)
        Point.y--;

    imgSize = width * height;
    x = Point.x;
    y = Point.y;

    imgdata[y * width + x] = color.Y;
    imgdata[imgSize + y * width / 2 + x] = color.U;
    imgdata[imgSize + y * width / 2 + x + 1] = color.V;

    return 0;
}

static int yuv420_draw_vline(void* imgdata,
    int width,
    int height,
    YUV_Point startPoint,
    YUV_Point endPoint,
    YUV_Color color)
{
    int ret = 0;
    int x0, y0, y1;
    YUV_Point Point;

    x0 = startPoint.x;
    y0 = startPoint.y;
    y1 = endPoint.y;

    if (y0 > y1)
        YUV_SWAP(&y0, &y1);

    Point.x = x0;
    Point.y = y0;

    while (Point.y < y1) {
        Point.y++;
        ret = yuv420_draw_point((unsigned char *)imgdata,
            width, height, Point, color);
        if (ret)
            return -1;
    }

    return 0;
}

static int yuv420_draw_hline(void* imgdata,
    int width,
    int height,
    YUV_Point startPoint,
    YUV_Point endPoint,
    YUV_Color color)
{
    int ret = 0;
    int x0, y0, x1;
    YUV_Point Point;

    x0 = startPoint.x;
    y0 = startPoint.y;
    x1 = endPoint.x;

    if (x0 > x1)
        YUV_SWAP(&x0, &x1);

    Point.x = x0;
    Point.y = y0;

    while (Point.x < x1) {
        Point.x++;
        ret = yuv420_draw_point((unsigned char *)imgdata,
            width, height, Point, color);
        if (ret)
            return -1;
    }

    return 0;
}

static int yuv420_draw_tline(void* imgdata,
    int width,
    int height,
    YUV_Point startPoint,
    YUV_Point endPoint,
    YUV_Color color)
{
    int x0, y0, x1, y1;
    int dx, dy, error;
    YUV_Point Point;

    x0 = startPoint.x;
    y0 = startPoint.y;
    x1 = endPoint.x;
    y1 = endPoint.y;

    if (x0 > x1) {
        YUV_SWAP(&x0, &x1);
        YUV_SWAP(&y0, &y1);
    }

    dx = x1 - x0;
    dy = y1 - y0;
    Point.x = x0;
    Point.y = y0;

    if (dx == 0) {
        yuv420_draw_vline(imgdata, width,
            height, startPoint, endPoint, color);
        return 0;
    }
    if (dy == 0){
        yuv420_draw_hline(imgdata, width,
            height, startPoint, endPoint, color);
        return 0;
    }

    yuv420_draw_point((unsigned char *)imgdata,
        width, height, Point, color);

    if ((dx >= dy) && (dy > 0)) {/* 0<k<=1 */
        error = (dy*2) - dx;
        while (Point.x < x1) {
            if (error > 0) {
                error += (dy - dx) * 2;
                Point.x++;
                Point.y++;
            } else {
                error += dy * 2;
                Point.x++;
            }
            yuv420_draw_point((unsigned char *)imgdata,
                width, height, Point, color);
        }
        return 0;
    }

    if ((dy > dx) && (dy > 0)) {/* k>1 */
        error = dy - (dx * 2);
        while (Point.y < y1) {
            if (error < 0) {
                error += (dy - dx) * 2;
                Point.x++;
                Point.y++;
            } else {
                error += (-dx) * 2;
                Point.y++;
            }
            yuv420_draw_point((unsigned char *)imgdata,
                width, height, Point, color);
        }
        return 0;
    }

    if ((dx >= ABS(dy)) && (dy < 0)) { /* -1=<k<0 */
        error = (dy * 2) + dx;
        while (Point.x < x1) {
            if (error < 0) {
                error += (dy + dx) * 2;
                Point.x++;
                Point.y--;
            } else {
                error += dy * 2;
                Point.x++;
            }
            yuv420_draw_point((unsigned char *)imgdata,
                width, height, Point, color);
        }
        return 0;
    }

    if ((ABS(dy) > dx) && (dy < 0)) { /* k<-1 */
        error = dy + (dx * 2);
        while (Point.y > y1) {
            if (error > 0) {
                error += (dy + dx) * 2;
                Point.x++;
                Point.y--;
            } else {
                error += dx * 2;
                Point.y--;
            }
            yuv420_draw_point((unsigned char *)imgdata,
                width, height, Point, color);
        }
        return 0;
    }

    return 0;
}

void yuv420_draw_line(void* imgdata,
    int width,
    int height,
    YUV_Point startPoint,
    YUV_Point endPoint,
    YUV_Color color)
{
    YUV_Point sPoint;
    YUV_Point ePoint;

    sPoint.x = startPoint.x - 2;
    sPoint.y = startPoint.y - 2;
    ePoint.x = endPoint.x - 2;
    ePoint.y = endPoint.y - 2;
    yuv420_draw_tline(imgdata, width, height, sPoint, ePoint, color);

    sPoint.x = startPoint.x;
    sPoint.y = startPoint.y;
    ePoint.x = endPoint.x;
    ePoint.y = endPoint.y;
    yuv420_draw_tline(imgdata, width, height, sPoint, ePoint, color);

    sPoint.x = startPoint.x + 2;
    sPoint.y = startPoint.y + 2;
    ePoint.x = endPoint.x + 2;
    ePoint.y = endPoint.y + 2;
    yuv420_draw_tline(imgdata, width, height, sPoint, ePoint, color);
}

void yuv420_draw_rectangle(void* imgdata,
    int width,
    int height,
    YUV_Rect rect_rio,
    YUV_Color color)
{
    int RoiWidth = rect_rio.width;
    int RoiHeight = rect_rio.height;
    int x = rect_rio.x;
    int y = rect_rio.y;
    YUV_Point  Point[4];

    Point[0].x = x;
    Point[0].y = y;
    Point[1].x = x + RoiWidth - 2;
    Point[1].y = y;
    Point[2].x = x + RoiWidth - 2;
    Point[2].y = y + RoiHeight - 2;
    Point[3].x = x;
    Point[3].y = y + RoiHeight - 2;

    yuv420_draw_line(imgdata, width, height, Point[0], Point[1], color);
    yuv420_draw_line(imgdata, width, height, Point[1], Point[2], color);
    yuv420_draw_line(imgdata, width, height, Point[3], Point[2], color);
    yuv420_draw_line(imgdata, width, height, Point[0], Point[3], color);
}
