/*
 *  Copyright (c) 2019, Fuzhou Rockchip Electronics Co., Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifdef HAVE_DIX_CONFIG_H
#include "dix-config.h"
#endif

#include "exa.h"
#include "xf86.h"
#include "driver.h"
#include "dumb_bo.h"
#include "fbpict.h"

#include <unistd.h>

#ifdef MODESETTING_WITH_RGA
#include <rga/rga.h>
#include <rga/RgaApi.h>

#define RGA_MIN_LINEWIDTH       2

/* See rga_get_pixmap_format */
#define PIXMAP_IS_YUV(pix) ((pix)->drawable.bitsPerPixel == 12)
#endif

#define ABS(n)      ((n) < 0 ? -(n) : (n))
#define ANGLE(n)    ((n) < 0 ? (n) + 360 : (n))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))

typedef struct {
    struct dumb_bo *bo;
    int fd;
    int pitch;
    Bool owned;
} ms_exa_pixmap_priv;

typedef struct {
    struct {
        int alu;
        Pixel planemask;
        Pixel fg;
    } solid;
    struct {
        PixmapPtr pSrcPixmap;
        int alu;
        Pixel planemask;
    } copy;
    struct {
        int op;
        PicturePtr pSrcPicture;
        PicturePtr pMaskPicture;
        PicturePtr pDstPicture;
        PixmapPtr pSrc;
        PixmapPtr pMask;
        PixmapPtr pDst;

        int rotate;
        Bool reflect_y;
    } composite;
} ms_exa_prepare_args;

typedef struct {
    ms_exa_pixmap_priv *scratch_pixmap;
    ms_exa_prepare_args prepare_args;
} ms_exa_ctx;

#ifdef MODESETTING_WITH_RGA

static inline RgaSURF_FORMAT
rga_get_pixmap_format(PixmapPtr pPix)
{
    switch (pPix->drawable.bitsPerPixel) {
    case 32:
        if (pPix->drawable.depth == 32)
            return RK_FORMAT_BGRA_8888;
        return RK_FORMAT_BGRX_8888;
    case 12:
        switch (pPix->drawable.depth) {
        case 12:
            return RK_FORMAT_YCbCr_420_SP;
        /* HACK: Special depth for NV12_10 and NV16*/
        case 10:
            return RK_FORMAT_YCbCr_420_SP_10B;
        default:
            return RK_FORMAT_YCbCr_422_SP;
        }
    default:
        return RK_FORMAT_UNKNOWN;
    }
}

static Bool
rga_prepare_info(PixmapPtr pPixmap, rga_info_t *info,
                 int x, int y, int w, int h)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPixmap);
    RgaSURF_FORMAT format;
    int pitch, wstride, hstride;

    memset(info, 0, sizeof(rga_info_t));

    info->fd = -1;
    info->mmuFlag = 1;

    if (priv && priv->fd) {
        info->fd = priv->fd;
        pitch = priv->pitch;
    } else {
        info->virAddr = pPixmap->devPrivate.ptr;
        pitch = pPixmap->devKind;
    }

    format = rga_get_pixmap_format(pPixmap);

    /* rga requires yuv image rect align to 2 */
    if (PIXMAP_IS_YUV(pPixmap)) {
        x = (x + 1) & ~1;
        y = (y + 1) & ~1;
        w = w & ~1;
        h = h & ~1;
    }

    /* rga requires image width/height larger than 2 */
    if (w <= RGA_MIN_LINEWIDTH || h <= RGA_MIN_LINEWIDTH)
        return FALSE;

    wstride = pitch * 8 / pPixmap->drawable.bitsPerPixel;
    hstride = pPixmap->drawable.height;
    if (x < 0 || y < 0 || x + w > wstride || y + h > hstride)
        return FALSE;

    rga_set_rect(&info->rect, x, y, w, h, wstride, hstride, format);

    return TRUE;
}

static Bool
rga_check_pixmap(PixmapPtr pPixmap)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPixmap);
    RgaSURF_FORMAT format;

    /* rga requires image width/height larger than 2 */
    if (pPixmap->drawable.width <= RGA_MIN_LINEWIDTH &&
        pPixmap->drawable.height <= RGA_MIN_LINEWIDTH)
        return FALSE;

    format = rga_get_pixmap_format(pPixmap);
    if (format == RK_FORMAT_UNKNOWN)
        return FALSE;

    if (pPixmap->devKind && pPixmap->devPrivate.ptr)
        return TRUE;

    return priv && priv->fd;
}

#endif

static void ms_exa_done(PixmapPtr pPixmap) {}

Bool
ms_exa_prepare_access(PixmapPtr pPix, int index)
{
    ScreenPtr screen = pPix->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPix);

    if (pPix->devPrivate.ptr)
        return TRUE;

    if (!priv)
        return FALSE;

    dumb_bo_map(ms->drmmode.fd, priv->bo);

    pPix->devPrivate.ptr = priv->bo->ptr;

    return pPix->devPrivate.ptr != NULL;
}

void
ms_exa_finish_access(PixmapPtr pPix, int index)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPix);

    if (priv && priv->bo)
        pPix->devPrivate.ptr = NULL;
}

static Bool
ms_exa_prepare_solid(PixmapPtr pPixmap,
                     int alu, Pixel planemask, Pixel fg)
{
    ScreenPtr screen = pPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

#ifdef MODESETTING_WITH_RGA
    //int rop;

    if (planemask != ~0U)
        return FALSE;

    if (!rga_check_pixmap(pPixmap))
        return FALSE;

    /* TODO: Support rop */
    switch (alu) {
    case GXcopy:
        break;
    case GXclear:
    case GXset:
    case GXcopyInverted:
    default:
        return FALSE;
    }
#endif

    ctx->prepare_args.solid.alu = alu;
    ctx->prepare_args.solid.planemask = planemask;
    ctx->prepare_args.solid.fg = fg;

    return TRUE;
}

static void
ms_exa_solid_bail(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
    ScreenPtr screen = pPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    ChangeGCVal val[3];
    GCPtr gc;

    gc = GetScratchGC(pPixmap->drawable.depth, screen);

    val[0].val = ctx->prepare_args.solid.alu;
    val[1].val = ctx->prepare_args.solid.planemask;
    val[2].val = ctx->prepare_args.solid.fg;
    ChangeGC(NullClient, gc, GCFunction | GCPlaneMask | GCForeground, val);
    ValidateGC(&pPixmap->drawable, gc);

    ms_exa_prepare_access(pPixmap, 0);
    fbFill(&pPixmap->drawable, gc, x1, y1, x2 - x1, y2 - y1);
    ms_exa_finish_access(pPixmap, 0);

    FreeScratchGC(gc);
}

static void
ms_exa_solid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
#ifdef MODESETTING_WITH_RGA
    ScreenPtr screen = pPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    rga_info_t dst_info = {0};
    int width = x2 - x1;
    int height = y2 - y1;

    /* skip small images */
    if (width * height <= 4096)
        goto bail;

    if (!rga_prepare_info(pPixmap, &dst_info, x1, y1, x2 - x1, y2 - y1))
        goto bail;

    dst_info.color = ctx->prepare_args.solid.fg;

    /* rga only support RGBA8888 for color fill */
    if (pPixmap->drawable.bitsPerPixel != 32)
        goto bail;

    if (pPixmap->drawable.depth == 24)
        dst_info.color |= 0xFF << 24;

    dst_info.rect.format = RK_FORMAT_RGBA_8888;

    if (c_RkRgaColorFill(&dst_info) < 0)
        goto bail;

    return;

bail:
#endif

    ms_exa_solid_bail(pPixmap, x1, y1, x2, y2);
}

static Bool
ms_exa_prepare_copy(PixmapPtr pSrcPixmap,
                    PixmapPtr pDstPixmap,
                    int dx, int dy, int alu, Pixel planemask)
{
    ScreenPtr screen = pSrcPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

#ifdef MODESETTING_WITH_RGA
    //int rop;

    if (planemask != ~0U)
        return FALSE;

    if (!rga_check_pixmap(pSrcPixmap))
        return FALSE;

    if (!rga_check_pixmap(pDstPixmap))
        return FALSE;

    /* TODO: Support rop */
    switch (alu) {
    case GXcopy:
        break;
    case GXclear:
    case GXset:
    case GXcopyInverted:
    default:
        return FALSE;
    }
#endif

    ctx->prepare_args.copy.pSrcPixmap = pSrcPixmap;
    ctx->prepare_args.copy.alu = alu;
    ctx->prepare_args.copy.planemask = planemask;

    return TRUE;
}

static void
ms_exa_copy_bail(PixmapPtr pDstPixmap, int srcX, int srcY,
                 int dstX, int dstY, int width, int height)
{
    ScreenPtr screen = pDstPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PixmapPtr pSrcPixmap = ctx->prepare_args.copy.pSrcPixmap;
    ChangeGCVal val[2];
    GCPtr gc;

    gc = GetScratchGC(pDstPixmap->drawable.depth, screen);

    val[0].val = ctx->prepare_args.copy.alu;
    val[1].val = ctx->prepare_args.copy.planemask;
    ChangeGC(NullClient, gc, GCFunction | GCPlaneMask, val);
    ValidateGC(&pDstPixmap->drawable, gc);

    ms_exa_prepare_access(pSrcPixmap, 0);
    ms_exa_prepare_access(pDstPixmap, 0);
    fbCopyArea(&pSrcPixmap->drawable, &pDstPixmap->drawable, gc,
               srcX, srcY, width, height, dstX, dstY);
    ms_exa_finish_access(pDstPixmap, 0);
    ms_exa_finish_access(pSrcPixmap, 0);

    FreeScratchGC(gc);
}

static void
ms_exa_copy(PixmapPtr pDstPixmap, int srcX, int srcY,
            int dstX, int dstY, int width, int height)
{
#ifdef MODESETTING_WITH_RGA
    ScreenPtr screen = pDstPixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PixmapPtr pSrcPixmap = ctx->prepare_args.copy.pSrcPixmap;
    rga_info_t src_info = {0};
    rga_info_t dst_info = {0};
    rga_info_t tmp_info = {0};

    /* skip small images */
    if (width * height <= 4096)
        goto bail;

    if (!rga_prepare_info(pSrcPixmap, &src_info, srcX, srcY, width, height))
        goto bail;

    if (!rga_prepare_info(pDstPixmap, &dst_info, dstX, dstY, width, height))
        goto bail;

    /* need an extra buffer for overlap copy */
    if (pSrcPixmap == pDstPixmap &&
        (ABS(dstX - srcX) < width || ABS(dstY - srcX) < height)) {
        tmp_info.fd = ctx->scratch_pixmap->fd;
        tmp_info.mmuFlag = 1;

        rga_set_rect(&tmp_info.rect, 0, 0, width, height,
                     width, height, rga_get_pixmap_format(pDstPixmap));

        if (c_RkRgaBlit(&src_info, &tmp_info, NULL) < 0)
            goto bail;

        src_info = tmp_info;
    }

    if (c_RkRgaBlit(&src_info, &dst_info, NULL) < 0)
        goto bail;

    return;

bail:
#endif

    ms_exa_copy_bail(pDstPixmap, srcX, srcY, dstX, dstY, width, height);
}

static Bool
ms_exa_check_composite(int op,
                       PicturePtr pSrcPicture,
                       PicturePtr pMaskPicture, PicturePtr pDstPicture)
{
#ifdef MODESETTING_WITH_RGA
    /* TODO: Support other op */
    if (op != PictOpSrc && op != PictOpOver)
        return FALSE;

    /* TODO: Support mask */
    if (pMaskPicture)
        return FALSE;

    /* TODO: Multiply transform from src and dst */
    if (pDstPicture->transform)
        return FALSE;
#endif

    if (!pSrcPicture->pDrawable)
        return FALSE;

    return TRUE;
}

static Bool
ms_exa_parse_transform(PictTransformPtr t, int *rotate, Bool *reflect_y)
{
    PictVector v;
    double x, y, dx, dy;
    int r1, r2;

    if (!t) {
        *rotate = 0;
        *reflect_y = FALSE;
        return TRUE;
    }

    /* Only support affine matrix */
    if (t->matrix[2][0] || t->matrix[2][1] || !t->matrix[2][2])
        return FALSE;

    dx = t->matrix[0][2] / (double) t->matrix[2][2];
    dy = t->matrix[1][2] / (double) t->matrix[2][2];

    v.vector[0] = IntToxFixed(1);
    v.vector[1] = IntToxFixed(0);
    v.vector[2] = xFixed1;
    PictureTransformPoint(t, &v);
    x = pixman_fixed_to_double(v.vector[0]) - dx;
    y = pixman_fixed_to_double(v.vector[1]) - dy;
    r1 = (int) ANGLE(atan2(y, x) * 180 / M_PI);

    /* Only support 0/90/180/270 rotations */
    if (r1 % 90)
        return FALSE;

    v.vector[0] = IntToxFixed(0);
    v.vector[1] = IntToxFixed(1);
    v.vector[2] = xFixed1;
    PictureTransformPoint(t, &v);
    x = pixman_fixed_to_double(v.vector[0]) - dx;
    y = pixman_fixed_to_double(v.vector[1]) - dy;
    r2 = (int) ANGLE(atan2(y, x) * 180 / M_PI - 90);

    *rotate = (360 - r1) % 360;
    *reflect_y = r1 != r2;

    return TRUE;
}

static Bool
ms_exa_prepare_composite(int op,
                         PicturePtr pSrcPicture,
                         PicturePtr pMaskPicture,
                         PicturePtr pDstPicture,
                         PixmapPtr pSrc, PixmapPtr pMask, PixmapPtr pDst)
{
    ScreenPtr screen = pSrc->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

#ifdef MODESETTING_WITH_RGA
    PictTransformPtr t = pSrcPicture->transform;

    if (!rga_check_pixmap(pSrc))
        return FALSE;

    if (!rga_check_pixmap(pDst))
        return FALSE;

    if (pDst == pSrc)
        return FALSE;

    /* TODO: Support repeat */
    if (pSrcPicture->repeat)
        return FALSE;

    /* TODO: Handle pSrcPicture->filter */

    if (!ms_exa_parse_transform(t, &ctx->prepare_args.composite.rotate,
                                &ctx->prepare_args.composite.reflect_y))
        return FALSE;
#endif
    ctx->prepare_args.composite.op = op;
    ctx->prepare_args.composite.pSrcPicture = pSrcPicture;
    ctx->prepare_args.composite.pMaskPicture = pMaskPicture;
    ctx->prepare_args.composite.pDstPicture = pDstPicture;
    ctx->prepare_args.composite.pSrc = pSrc;
    ctx->prepare_args.composite.pMask = pMask;

    return TRUE;
}

static inline void
ms_exa_composite_fix_offsets(DrawablePtr pDrawable, PixmapPtr pPix,
                             int *xoff, int *yoff)
{
    // Base on fb/fb.h#fbGetDrawablePixmap
    if (pDrawable->type != DRAWABLE_PIXMAP) {
        ScreenPtr pScreen = pDrawable->pScreen;
        pPix = pScreen->GetWindowPixmap((WindowPtr) pDrawable);

#ifdef COMPOSITE
        *xoff -= pPix->drawable.x - pPix->screen_x;
        *yoff -= pPix->drawable.y - pPix->screen_y;
#else
        *xoff -= pPix->drawable.x;
        *yoff -= pPix->drawable.y;
#endif
    } else {
        *xoff -= pDrawable->x;
        *yoff -= pDrawable->y;
    }

    // Based on fb/fbpict.c#create_bits_picture
    *xoff -= pDrawable->x;
    *yoff -= pDrawable->y;
}

static void
ms_exa_composite_bail(PixmapPtr pDst, int srcX, int srcY,
                      int maskX, int maskY, int dstX, int dstY,
                      int width, int height)
{
    ScreenPtr screen = pDst->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PicturePtr pSrcPicture = ctx->prepare_args.composite.pSrcPicture;
    PicturePtr pMaskPicture = ctx->prepare_args.composite.pMaskPicture;
    PicturePtr pDstPicture = ctx->prepare_args.composite.pDstPicture;
    PixmapPtr pSrc = ctx->prepare_args.composite.pSrc;
    PixmapPtr pMask = ctx->prepare_args.composite.pMask;
    int op = ctx->prepare_args.composite.op;

    if (pMask)
        ms_exa_prepare_access(pMask, 0);

    ms_exa_prepare_access(pSrc, 0);
    ms_exa_prepare_access(pDst, 0);

    ms_exa_composite_fix_offsets(pSrcPicture->pDrawable, pSrc, &srcX, &srcY);
    ms_exa_composite_fix_offsets(pDstPicture->pDrawable, pDst, &dstX, &dstY);
    if (pMaskPicture && pMask)
        ms_exa_composite_fix_offsets(pMaskPicture->pDrawable,
                                     pMask, &maskX, &maskY);

    fbComposite(op, pSrcPicture, pMaskPicture, pDstPicture,
                srcX, srcY, maskX, maskY,
                dstX, dstY, width, height);

    ms_exa_finish_access(pDst, 0);
    ms_exa_finish_access(pSrc, 0);

    if (pMask)
        ms_exa_finish_access(pMask, 0);
}

static void
ms_exa_composite(PixmapPtr pDst, int srcX, int srcY,
                 int maskX, int maskY, int dstX, int dstY,
                 int width, int height)
{
#ifdef MODESETTING_WITH_RGA
    ScreenPtr screen = pDst->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PictTransformPtr t = ctx->prepare_args.composite.pSrcPicture->transform;
    PixmapPtr pSrc = ctx->prepare_args.composite.pSrc;
    BoxRec box = {
        .x1 = srcX,
        .y1 = srcY,
        .x2 = srcX + width,
        .y2 = srcY + height,
    };
    rga_info_t src_info = {0};
    rga_info_t dst_info = {0};
    rga_info_t tmp_info = {0};
    Bool reflect_y = ctx->prepare_args.composite.reflect_y;
    int rotate = ctx->prepare_args.composite.rotate;
    int op = ctx->prepare_args.composite.op;
    int sw, sh, blend = 0;

    /* skip small images */
    if (width * height <= 4096)
        goto bail;

    if (t)
        pixman_transform_bounds(t, &box);

    sw = box.x2 - box.x1;
    sh = box.y2 - box.y1;

    /* rga has scale limits */
    if ((double)sw / width > 16 || (double)width / sw > 16 ||
        (double)sh / height > 16 || (double)height / sh > 16)
        goto bail;

    if (!rga_prepare_info(pSrc, &src_info, box.x1, box.y1, sw, sh))
        goto bail;

    if (!rga_prepare_info(pDst, &dst_info, dstX, dstY, width, height))
        goto bail;

    /* dst = src + (1 - src.a) * dst */
    if (op == PictOpOver)
        blend = 0xFF0405;

    if (rotate == 90)
        src_info.rotation = HAL_TRANSFORM_ROT_90;
    else if (rotate == 180)
        src_info.rotation = HAL_TRANSFORM_ROT_180;
    else if (rotate == 270)
        src_info.rotation = HAL_TRANSFORM_ROT_270;

    /* need an extra buffer for reflect + rotate composite */
    if (reflect_y && rotate) {
        tmp_info.fd = ctx->scratch_pixmap->fd;
        tmp_info.mmuFlag = 1;

        rga_set_rect(&tmp_info.rect, 0, 0, width, height,
                     width, height, rga_get_pixmap_format(pDst));

        src_info.blend = blend;
        if (c_RkRgaBlit(&src_info, &tmp_info, NULL) < 0)
            goto bail;

        src_info = tmp_info;
    }

    if (reflect_y)
        src_info.rotation = HAL_TRANSFORM_FLIP_V;

    src_info.blend = blend;
    if (c_RkRgaBlit(&src_info, &dst_info, NULL) < 0)
        goto bail;

    return;

bail:
#endif

    ms_exa_composite_bail(pDst, srcX, srcY, maskX, maskY,
                          dstX, dstY, width, height);
}

static Bool
ms_exa_upload_to_screen(PixmapPtr pDst, int x, int y, int w, int h,
                        char *src, int src_pitch)
{
#ifndef MODESETTING_WITH_RGA
    return FALSE;
#else
    ScreenPtr screen = pDst->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PixmapPtr pixmap;
    Bool ret = FALSE;

    /* rga requires image width/height larger than 2 */
    if (w <= RGA_MIN_LINEWIDTH || h <= RGA_MIN_LINEWIDTH)
        return FALSE;

    /* skip small images */
    if (w * h <= 4096)
        return FALSE;

    if (!rga_check_pixmap(pDst))
        return FALSE;

    /* copy the h-1 lines */
    h -= 1;
    pixmap = drmmode_create_pixmap_header(screen, w, h,
                                          pDst->drawable.depth,
                                          pDst->drawable.bitsPerPixel,
                                          src_pitch, NULL);
    if (!pixmap)
        goto out;

    pixmap->devKind = src_pitch;
    pixmap->devPrivate.ptr = src;
    ctx->prepare_args.copy.pSrcPixmap = pixmap;
    ms_exa_copy(pDst, 0, 0, x, y, w, h);

    screen->DestroyPixmap(pixmap);

    /* copy the last line separately */
    pixmap = drmmode_create_pixmap_header(screen, w, 1,
                                          pDst->drawable.depth,
                                          pDst->drawable.bitsPerPixel,
                                          src_pitch, NULL);
    if (!pixmap)
        goto out;

    pixmap->devKind = w * pDst->drawable.bitsPerPixel / 8;
    pixmap->devPrivate.ptr = src + src_pitch * h;
    ctx->prepare_args.copy.pSrcPixmap = pixmap;
    ms_exa_copy(pDst, 0, 0, x, y + h, w, 1);

    ret = TRUE;

out:
    if (pixmap)
        screen->DestroyPixmap(pixmap);

    return ret;
#endif
}

static Bool
ms_exa_download_from_screen(PixmapPtr pSrc, int x, int y, int w, int h,
                            char *dst, int dst_pitch)
{
#ifndef MODESETTING_WITH_RGA
    return FALSE;
#else
    ScreenPtr screen = pSrc->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_ctx *ctx = ms->drmmode.exa->priv;

    PixmapPtr pixmap;
    Bool ret = FALSE;

    /* rga requires image width/height larger than 2 */
    if (w <= RGA_MIN_LINEWIDTH || h <= RGA_MIN_LINEWIDTH)
        return FALSE;

    /* skip small images */
    if (w * h <= 4096)
        return FALSE;

    if (!rga_check_pixmap(pSrc))
        return FALSE;

    ctx->prepare_args.copy.pSrcPixmap = pSrc;

    /* copy the h-1 lines */
    h -= 1;
    pixmap = drmmode_create_pixmap_header(screen, w, h,
                                          pSrc->drawable.depth,
                                          pSrc->drawable.bitsPerPixel,
                                          dst_pitch, NULL);
    if (!pixmap)
        goto out;

    pixmap->devKind = dst_pitch;
    pixmap->devPrivate.ptr = dst;
    ms_exa_copy(pixmap, x, y, 0, 0, w, h);

    screen->DestroyPixmap(pixmap);

    /* copy the last line separately */
    pixmap = drmmode_create_pixmap_header(screen, w, 1,
                                          pSrc->drawable.depth,
                                          pSrc->drawable.bitsPerPixel,
                                          dst_pitch, NULL);
    if (!pixmap)
        goto out;

    pixmap->devKind = w * pSrc->drawable.bitsPerPixel / 8;
    pixmap->devPrivate.ptr = dst + dst_pitch * h;
    ms_exa_copy(pixmap, x, y + h, 0, 0, w, 1);

    ret = TRUE;

out:
    if (pixmap)
        screen->DestroyPixmap(pixmap);

    return ret;
#endif
}

static void
ms_exa_wait_marker(ScreenPtr pScreen, int marker)
{
    // TODO: Use async rga, and sync for specified request here.
}

static int
ms_exa_mark_sync(ScreenPtr pScreen)
{
    // TODO: return latest request(marker).
    return 0;
}

static void
ms_exa_destroy_pixmap(ScreenPtr pScreen, void *driverPriv)
{
    ScrnInfoPtr scrn = xf86Screens[pScreen->myNum];
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_pixmap_priv *priv = driverPriv;

    if (priv->fd > 0)
        close(priv->fd);

    if (priv->owned && priv->bo)
        dumb_bo_destroy(ms->drmmode.fd, priv->bo);

    free(priv);
}

static void *
ms_exa_create_pixmap2(ScreenPtr pScreen, int width, int height,
                      int depth, int usage_hint, int bitsPerPixel,
                      int *new_fb_pitch)
{
    ScrnInfoPtr scrn = xf86Screens[pScreen->myNum];
    modesettingPtr ms = modesettingPTR(scrn);
    ms_exa_pixmap_priv *priv;

    priv = calloc(1, sizeof(ms_exa_pixmap_priv));
    if (!priv)
        return NULL;

    if (!width && !height)
        return priv;

    priv->bo = dumb_bo_create(ms->drmmode.fd, width, height, bitsPerPixel);
    if (!priv->bo)
        goto fail;

    priv->owned = TRUE;

    priv->fd = dumb_bo_get_fd(ms->drmmode.fd, priv->bo, 0);
    priv->pitch = priv->bo->pitch;

    if (new_fb_pitch)
        *new_fb_pitch = priv->pitch;

    return priv;

fail:
    free(priv);
    return NULL;
}

static Bool
ms_exa_pixmap_is_offscreen(PixmapPtr pPixmap)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPixmap);

    return priv && priv->bo;
}

Bool
ms_exa_set_pixmap_bo(ScrnInfoPtr scrn, PixmapPtr pPixmap,
                     struct dumb_bo *bo, Bool owned)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pPixmap);
    modesettingPtr ms = modesettingPTR(scrn);

    if (!ms->drmmode.exa || !priv)
        return FALSE;

    if (priv->fd > 0)
        close(priv->fd);

    if (priv->owned && priv->bo)
        dumb_bo_destroy(ms->drmmode.fd, priv->bo);

    priv->bo = bo;
    priv->fd = dumb_bo_get_fd(ms->drmmode.fd, priv->bo, 0);
    priv->pitch = priv->bo->pitch;

    priv->owned = owned;

    pPixmap->devPrivate.ptr = NULL;
    pPixmap->devKind = priv->pitch;

    return TRUE;
}

struct dumb_bo *
ms_exa_bo_from_pixmap(ScreenPtr screen, PixmapPtr pixmap)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pixmap);
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);

    if (!ms->drmmode.exa || !priv)
        return NULL;

    return priv->bo;
}

void
ms_exa_exchange_buffers(PixmapPtr front, PixmapPtr back)
{
    ms_exa_pixmap_priv *front_priv = exaGetPixmapDriverPrivate(front);
    ms_exa_pixmap_priv *back_priv = exaGetPixmapDriverPrivate(back);
    ms_exa_pixmap_priv tmp_priv;

    tmp_priv = *front_priv;
    *front_priv = *back_priv;
    *back_priv = tmp_priv;
}

Bool
ms_exa_back_pixmap_from_fd(PixmapPtr pixmap,
                           int fd,
                           CARD16 width,
                           CARD16 height,
                           CARD16 stride, CARD8 depth, CARD8 bpp)
{
    ScreenPtr screen = pixmap->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    struct dumb_bo *bo;
    Bool ret;

    bo = dumb_get_bo_from_fd(ms->drmmode.fd, fd,
                             stride, stride * height);
    if (!bo)
        return FALSE;

    screen->ModifyPixmapHeader(pixmap, width, height,
                               depth, bpp, stride, NULL);

    ret = ms_exa_set_pixmap_bo(scrn, pixmap, bo, TRUE);
    if (!ret)
        dumb_bo_destroy(ms->drmmode.fd, bo);

    return ret;
}

int
ms_exa_shareable_fd_from_pixmap(ScreenPtr screen,
                                PixmapPtr pixmap,
                                CARD16 *stride,
                                CARD32 *size)
{
    ms_exa_pixmap_priv *priv = exaGetPixmapDriverPrivate(pixmap);
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);

    if (!ms->drmmode.exa || !priv || !priv->fd)
        return -1;

    return priv->fd;
}

Bool
ms_exa_copy_area(PixmapPtr pSrc, PixmapPtr pDst,
                 pixman_f_transform_t *transform, RegionPtr clip)
{
#ifdef MODESETTING_WITH_RGA
    ScreenPtr screen = pSrc->drawable.pScreen;
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    rga_info_t src_info = {0};
    rga_info_t dst_info = {0};
    RegionPtr region = NULL;
    BoxPtr box;
    int n;

    if (!ms->drmmode.exa)
        goto bail;

    if (!rga_check_pixmap(pSrc))
        goto bail;

    if (!rga_check_pixmap(pDst))
        goto bail;

    /* Fallback to compositor for rotate / reflect */
    if (transform) {
        pixman_transform_t t;
        Bool reflect_y = FALSE;
        int rotate = 0;

        pixman_transform_from_pixman_f_transform(&t, transform);
        if (!ms_exa_parse_transform(&t, &rotate, &reflect_y))
            goto bail;

        if (rotate || reflect_y)
            goto bail;
    }

    region = RegionDuplicate(clip);
    n = REGION_NUM_RECTS(region);

    while(n--) {
        int sx, sy, sw, sh, dx, dy, dw, dh;
        box = REGION_RECTS(region) + n;

        box->x1 = max(box->x1, 0);
        box->y1 = max(box->y1, 0);
        box->x2 = min(box->x2, pDst->drawable.width);
        box->y2 = min(box->y2, pDst->drawable.height);

        dx = box->x1;
        dy = box->y1;
        dw = box->x2 - box->x1;
        dh = box->y2 - box->y1;

        if (transform)
            pixman_f_transform_bounds(transform, box);

        sx = max(box->x1, 0);
        sy = max(box->y1, 0);
        sw = min(box->x2, pSrc->drawable.width) - sx;
        sh = min(box->y2, pSrc->drawable.height) - sy;

        /* rga has scale limits */
        if ((double)sw / dw > 16 || (double)dw / sw > 16 ||
            (double)sh / dh > 16 || (double)dh / sh > 16)
            goto err;

        if (!rga_prepare_info(pSrc, &src_info, sx, sy, sw, sh))
            goto err;

        if (!rga_prepare_info(pDst, &dst_info, dx, dy, dw, dh))
            goto err;

        if (!c_RkRgaBlit(&src_info, &dst_info, NULL))
            continue;
err:
        /* HACK: Ignoring errors for YUV, since xserver cannot handle it */
        if (!PIXMAP_IS_YUV(pSrc) && !PIXMAP_IS_YUV(pDst))
            goto bail;
    }

    RegionDestroy(region);
    return TRUE;

bail:
    if (region)
        RegionDestroy(region);
#endif

    return ms_copy_area(pSrc, pDst, transform, clip);
}

static inline void
ms_setup_exa(ExaDriverPtr exa)
{
    exa->exa_major = EXA_VERSION_MAJOR;
    exa->exa_minor = EXA_VERSION_MINOR;

    exa->pixmapPitchAlign = 8;
    exa->flags = EXA_OFFSCREEN_PIXMAPS;
    exa->maxX = 4096;
    exa->maxY = 4096;

    exa->PrepareSolid = ms_exa_prepare_solid;
    exa->Solid = ms_exa_solid;
    exa->DoneSolid = ms_exa_done;

    exa->PrepareCopy = ms_exa_prepare_copy;
    exa->Copy = ms_exa_copy;
    exa->DoneCopy = ms_exa_done;

    exa->CheckComposite = ms_exa_check_composite;
    exa->PrepareComposite = ms_exa_prepare_composite;
    exa->Composite = ms_exa_composite;
    exa->DoneComposite = ms_exa_done;

    /* Disable upload/download, until rga2 crash issue fixed */
    exa->UploadToScreen = ms_exa_upload_to_screen;
    exa->DownloadFromScreen = ms_exa_download_from_screen;

    exa->WaitMarker = ms_exa_wait_marker;
    exa->MarkSync = ms_exa_mark_sync;

    // bo based pixmap ops
    exa->flags |= EXA_HANDLES_PIXMAPS | EXA_SUPPORTS_PREPARE_AUX;

    exa->DestroyPixmap = ms_exa_destroy_pixmap;
    exa->CreatePixmap2 = ms_exa_create_pixmap2;
    exa->PrepareAccess = ms_exa_prepare_access;
    exa->FinishAccess = ms_exa_finish_access;
    exa->PixmapIsOffscreen = ms_exa_pixmap_is_offscreen;
}

Bool
ms_init_exa(ScrnInfoPtr scrn)
{
    modesettingPtr ms = modesettingPTR(scrn);
    ScreenPtr screen = scrn->pScreen;
    drmmode_exa *exa;
    ms_exa_ctx *ctx;

    if (ms->drmmode.exa)
        ms_deinit_exa(scrn);

#ifdef MODESETTING_WITH_RGA
    if (c_RkRgaInit() < 0)
        return FALSE;

    xf86DrvMsg(scrn->scrnIndex, X_INFO, "Using RGA EXA\n");
#else
    xf86DrvMsg(scrn->scrnIndex, X_INFO, "Using software EXA\n");
#endif

    ms->drmmode.exa = calloc(1, sizeof(drmmode_exa));
    if (!ms->drmmode.exa)
        return FALSE;

    exa = ms->drmmode.exa;
    exa->driver = exaDriverAlloc();
    if (!exa->driver)
        goto bail;

    ms_setup_exa(exa->driver);

    if (!exaDriverInit(screen, exa->driver))
        goto bail;

    exa->priv = calloc(1, sizeof(ms_exa_ctx));
    if (!exa->priv)
        goto bail;

    ctx = exa->priv;
    ctx->scratch_pixmap = ms_exa_create_pixmap2(screen,
                                         exa->driver->maxX, exa->driver->maxY,
                                         scrn->depth, 0,
                                         scrn->bitsPerPixel,
                                         NULL);
    if (!ctx->scratch_pixmap)
        goto bail;

    return TRUE;

bail:
    ms_deinit_exa(scrn);
    return FALSE;
}

void
ms_deinit_exa(ScrnInfoPtr scrn)
{
    modesettingPtr ms = modesettingPTR(scrn);
    ScreenPtr screen = scrn->pScreen;
    drmmode_exa *exa;
    ms_exa_ctx *ctx;

    if (!ms->drmmode.exa)
        return;

    exa = ms->drmmode.exa;
    ctx = exa->priv;

    if (ctx) {
        if (ctx->scratch_pixmap)
            ms_exa_destroy_pixmap(screen, ctx->scratch_pixmap);

        free(ctx);
    }

    if (exa->driver) {
        exaDriverFini(screen);
        free(exa->driver);
    }

    free(exa);
    ms->drmmode.exa = NULL;

#ifdef MODESETTING_WITH_RGA
    c_RkRgaDeInit();
#endif
}
