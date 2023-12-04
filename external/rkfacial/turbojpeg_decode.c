/*
 * Copyright (C) 2020 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
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
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <turbojpeg.h>

#define ALIGN(x, a) ((x) & ~((a)-1))

const char *subsampName[TJ_NUMSAMP] = {
    "4:4:4", "4:2:2", "4:2:0", "Grayscale", "4:4:0", "4:1:1"
};

const char *colorspaceName[TJ_NUMCS] = {
    "RGB", "YCbCr", "GRAY", "CMYK", "YCCK"
};

void *turbojpeg_decode_get(const char *name, int *w, int *h, int *b)
{
    FILE *fp;
    size_t size;
    unsigned char *jpeg = NULL;
    unsigned char *rgb = NULL;
    unsigned char *rgb_align = NULL;
    tjhandle handle;
    tjscalingfactor scalingFactor = { 1, 1 };
    int width, height;
    int inSubsamp, inColorspace;
    int pixelFormat;
    int flags = 0;
    int dst_index, src_index;

    fp = fopen(name, "rb");
    if (!fp) {
        printf("open %s failed!\n", name);
        goto err0;
    }
    if (fseek(fp, 0, SEEK_END) < 0 || ((size = ftell(fp)) < 0) ||
            fseek(fp, 0, SEEK_SET) < 0) {
        printf("%s: determining input file size\n", name);
        goto err1;
    }
    if (size == 0) {
        printf("%s: Input file contains no data\n", name);
        goto err1;
    }

    jpeg = (unsigned char *)tjAlloc(size);
    if (!jpeg) {
        printf("allocating JPEG buffer failed\n");
        goto err1;
    }

    if (fread(jpeg, size, 1, fp) < 1) {
        printf("reading input file failed\n");
        goto err2;
    }

    if ((handle = tjInitDecompress()) == NULL) {
        printf("initializing decompressor failed\n");
        goto err2;
    }

    if (tjDecompressHeader3(handle, jpeg, size, &width, &height,
                &inSubsamp, &inColorspace) < 0) {
        printf("reading JPEG header failed\n");
        goto err3;
    }

    //printf("%d x %d pixels, %s subsampling, %s colorspace\n",
    //        width, height, subsampName[inSubsamp], colorspaceName[inColorspace]);

    width = TJSCALED(width, scalingFactor);
    height = TJSCALED(height, scalingFactor);

    pixelFormat = TJPF_RGB;
    if ((rgb = (unsigned char*)tjAlloc(width * height * tjPixelSize[pixelFormat])) == NULL) {
        printf("allocating rgb buffer failed\n");
        goto err3;
    }

    if (tjDecompress2(handle, jpeg, size, rgb, width, 0, height, pixelFormat, flags) < 0) {
        printf("decompressing JPEG image failed\n");
        goto err4;
    }

    *w = ALIGN(width, 4);
    *h = ALIGN(height, 4);
    *b = tjPixelSize[pixelFormat];
    /* Not need Align */
    if (*w == width)
        goto err3;

    if ((rgb_align = (unsigned char*)tjAlloc((*w) * (*h) * (*b))) == NULL) {
        printf("allocating rgb_align buffer failed\n");
        goto err4;
    }

    /* Align for RGA */
    for (int i = 0; i < (*h); i++) {
        dst_index = i * (*w) * (*b);
        src_index = i * width * (*b);
        memcpy(rgb_align + dst_index, rgb + src_index, (*w) * (*b));
    }

err4:
    if (rgb) {
        tjFree(rgb);
        rgb = NULL;
    }

err3:
    if (handle)
        tjDestroy(handle);

err2:
    if (jpeg)
        tjFree(jpeg);

err1:
    if (fp)
        fclose(fp);

err0:
    return rgb ? rgb : rgb_align;
}

void turbojpeg_decode_put(void *data)
{
    if (data)
        tjFree(data);
}
