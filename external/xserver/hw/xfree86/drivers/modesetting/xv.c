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

#include "driver.h"
#include "dumb_bo.h"
/*
#include "xf86.h"
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <libdrm/drm_fourcc.h>

#include <X11/extensions/Xv.h>
#include "fourcc.h"

#ifndef DRM_FORMAT_NV12_10
#define DRM_FORMAT_NV12_10 fourcc_code('N', 'A', '1', '2')
#endif

#define XVIMAGE_XRGB8888 \
   { \
        DRM_FORMAT_XRGB8888, \
        XvRGB, \
        LSBFirst, \
        {'R','G','B','X', \
          0x00,0x00,0x00,0x10,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71}, \
        32, \
        XvPacked, \
        1, \
        24, 0xff0000, 0x00ff00, 0x0000ff, \
        0, 0, 0, \
        0, 0, 0, \
        0, 0, 0, \
        {'B','G','R', \
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, \
        XvTopToBottom \
   }

#define NUM_FORMATS 4

static XF86VideoFormatRec Formats[NUM_FORMATS] = {
    {15, TrueColor}, {16, TrueColor}, {24, TrueColor}, {30, TrueColor}
};

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

XvAttributeRec ms_exa_xv_attributes[] = {
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_CLIENT_ID"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_HOR_STRIDE"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_VER_STRIDE"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_DRM_FOURCC"},
    {0, 0, 0, NULL}
};
int ms_exa_xv_num_attributes = ARRAY_SIZE(ms_exa_xv_attributes) - 1;

Atom msDmaClient, msDmaHorStride, msDmaVerStride, msDmaDrmFourcc;

XvImageRec ms_exa_xv_images[] = {
    XVIMAGE_NV12,
    XVIMAGE_XRGB8888
};
int ms_exa_xv_num_images = ARRAY_SIZE(ms_exa_xv_images);

#define ALIGN(i,m) (((i) + (m) - 1) & ~((m) - 1))
#define ClipValue(v,min,max) ((v) < (min) ? (min) : (v) > (max) ? (max) : (v))

#define XV_MAX_DMA_FD 3

typedef struct {
    uint32_t dma_client;
    uint32_t dma_hor_stride;
    uint32_t dma_ver_stride;
    uint32_t dma_drm_fourcc;
    int dma_socket_fd;
} ms_exa_port_private;

static void
ms_exa_xv_set_dma_client(ms_exa_port_private *port_priv, uint32_t dma_client)
{
    struct sockaddr_un addr;

    // re-open socket to flush pending messages
    if (port_priv->dma_client)
        close(port_priv->dma_socket_fd);

    port_priv->dma_client = dma_client;

    if (!dma_client)
        goto clear;

    port_priv->dma_socket_fd = socket(PF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (port_priv->dma_socket_fd < 0)
        goto clear;

    addr.sun_family = AF_LOCAL;
    snprintf(addr.sun_path, sizeof(addr.sun_path),
             "/tmp/.xv_dma_client.%d", port_priv->dma_client);
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';

    unlink(addr.sun_path);
    if (bind(port_priv->dma_socket_fd,
             (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto clear;

    chmod(addr.sun_path, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);

    return;
clear:
    if (port_priv->dma_socket_fd > 0) {
        close(port_priv->dma_socket_fd);
        port_priv->dma_socket_fd = 0;
    }
    port_priv->dma_client = 0;
    port_priv->dma_hor_stride = 0;
    port_priv->dma_ver_stride = 0;
    port_priv->dma_drm_fourcc = 0;
}

static void
ms_exa_xv_stop_video(ScrnInfoPtr pScrn, void *data, Bool cleanup)
{
    ms_exa_port_private *port_priv = data;

    if (!cleanup)
        return;

    ms_exa_xv_set_dma_client(port_priv, 0);
}

static int
ms_exa_xv_set_port_attribute(ScrnInfoPtr pScrn,
                             Atom attribute, INT32 value, void *data)
{
    ms_exa_port_private *port_priv = data;

    if (attribute == msDmaClient)
        ms_exa_xv_set_dma_client(port_priv, ClipValue(value, 0, 0xFFFFFFFF));
    else if (attribute == msDmaHorStride)
        port_priv->dma_hor_stride = ClipValue(value, 0, 0xFFFFFFFF);
    else if (attribute == msDmaVerStride)
        port_priv->dma_ver_stride = ClipValue(value, 0, 0xFFFFFFFF);
    else if (attribute == msDmaDrmFourcc)
        port_priv->dma_drm_fourcc = ClipValue(value, 0, 0xFFFFFFFF);
    else
        return BadMatch;

    return Success;
}

static int
ms_exa_xv_get_port_attribute(ScrnInfoPtr pScrn,
                             Atom attribute, INT32 *value, void *data)
{
    ms_exa_port_private *port_priv = data;

    if (attribute == msDmaClient)
        *value = port_priv->dma_client;
    else if (attribute == msDmaHorStride)
        *value = port_priv->dma_hor_stride;
    else if (attribute == msDmaVerStride)
        *value = port_priv->dma_ver_stride;
    else if (attribute == msDmaDrmFourcc)
        *value = port_priv->dma_drm_fourcc;
    else
        return BadMatch;

    return Success;
}

static void
ms_exa_xv_query_best_size(ScrnInfoPtr pScrn,
                          Bool motion,
                          short vid_w, short vid_h,
                          short drw_w, short drw_h,
                          unsigned int *p_w, unsigned int *p_h, void *data)
{
    *p_w = drw_w;
    *p_h = drw_h;
}

static int
ms_exa_xv_query_image_attributes(ScrnInfoPtr pScrn,
                                 int id,
                                 unsigned short *w, unsigned short *h,
                                 int *pitches, int *offsets)
{
    int size = 0, tmp;

    if (offsets)
        offsets[0] = 0;

    if (id != FOURCC_NV12 && id != DRM_FORMAT_XRGB8888)
        return 0;

    *w = ALIGN(*w, 2);
    *h = ALIGN(*h, 2);
    size = ALIGN(*w, 4);
    if (pitches)
        pitches[0] = size;
    size *= *h;
    if (offsets)
        offsets[1] = size;
    tmp = ALIGN(*w, 4);
    if (id == DRM_FORMAT_XRGB8888)
        tmp *= 4;
    if (pitches)
        pitches[1] = tmp;
    tmp *= (*h >> 1);
    size += tmp;

    return size;
}

static PixmapPtr
ms_exa_xv_create_dma_pixmap(ScrnInfoPtr scrn,
                            ms_exa_port_private *port_priv, int id)
{
    modesettingPtr ms = modesettingPTR(scrn);
    ScreenPtr screen = scrn->pScreen;
    PixmapPtr pixmap = NULL;
    struct dumb_bo *bo = NULL;
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *header;
    char buf[CMSG_SPACE (sizeof (int))];
    int dma_fds[XV_MAX_DMA_FD], num_dma_fd = 0;
    int width, height, pitch, bpp, depth;
    uint32_t drm_fourcc;

    if (!port_priv->dma_client || port_priv->dma_socket_fd <= 0)
        return NULL;

    if (!port_priv->dma_hor_stride || !port_priv->dma_ver_stride)
        goto err;

    iov.iov_base = buf;
    iov.iov_len = 1;

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    num_dma_fd = 0;
    while (1) {
        msg.msg_control = buf;
        msg.msg_controllen = sizeof(buf);

        if (recvmsg(port_priv->dma_socket_fd, &msg, 0) < 0)
            break;

        /* End with a empty msg */
        header = CMSG_FIRSTHDR(&msg);
        if (!header)
            break;

        for (; header != NULL; header = CMSG_NXTHDR(&msg, header)) {
            if (header->cmsg_level != SOL_SOCKET
                || header->cmsg_type != SCM_RIGHTS
                || header->cmsg_len != CMSG_LEN(sizeof(int)))
                break;

            dma_fds[num_dma_fd++] = *((int *)CMSG_DATA(header));
        }
    }

    /* Only expect 1 buffer */
    if (num_dma_fd != 1)
        goto err;

    width = port_priv->dma_hor_stride;
    height = port_priv->dma_ver_stride;

    drm_fourcc = port_priv->dma_drm_fourcc ? port_priv->dma_drm_fourcc : id;
    if (drm_fourcc == DRM_FORMAT_XRGB8888) {
        pitch = width * 4;
        depth = bpp = 32;
    } else {
        pitch = width * 3 / 2;
        depth = bpp = 12;

        /* HACK: Special depth for NV12_10 and NV16*/
        if (drm_fourcc == DRM_FORMAT_NV12_10) {
            depth = 10;
        } else if (drm_fourcc == DRM_FORMAT_NV16) {
            depth = 16;
        }
    }

    pixmap = drmmode_create_pixmap_header(screen, width, height,
                                          depth, bpp, pitch, NULL);
    if (!pixmap)
        goto err;

    bo = dumb_get_bo_from_fd(ms->drmmode.fd, dma_fds[0],
                             pitch, pitch * height);
    if (!bo)
        goto err_free_pixmap;

    if (!ms_exa_set_pixmap_bo(scrn, pixmap, bo, TRUE))
        goto err_free_bo;

    goto out;

err_free_bo:
    dumb_bo_destroy(ms->drmmode.fd, bo);
err_free_pixmap:
    screen->DestroyPixmap(pixmap);
    pixmap = NULL;
err:
    ErrorF("ms xv failed to import dma pixmap\n");
    ms_exa_xv_set_dma_client(port_priv, 0);
out:
    while (num_dma_fd--)
        close(dma_fds[num_dma_fd]);

    return pixmap;
}

static PixmapPtr
ms_exa_xv_create_pixmap(ScrnInfoPtr scrn, ms_exa_port_private *port_priv,
                        int id,
                        unsigned char *buf,
                        short width,
                        short height)
{
    ScreenPtr screen = scrn->pScreen;
    PixmapPtr pixmap;
    int pitch, bpp;

    if (id == FOURCC_NV12) {
        pitch = ALIGN(width, 4) * 3 / 2;
        bpp = 12;
    } else {
        pitch = ALIGN(width, 4) * 4;
        bpp = 32;
    }

    pixmap = drmmode_create_pixmap_header(screen, width, height,
                                          bpp, bpp, pitch, buf);
    if (!pixmap)
        return NULL;

    pixmap->devKind = pitch;
    pixmap->devPrivate.ptr = buf;

    return pixmap;
}

static int
ms_exa_xv_put_image(ScrnInfoPtr pScrn,
                    short src_x, short src_y,
                    short drw_x, short drw_y,
                    short src_w, short src_h,
                    short drw_w, short drw_h,
                    int id,
                    unsigned char *buf,
                    short width,
                    short height,
                    Bool sync,
                    RegionPtr clipBoxes, void *data, DrawablePtr pDrawable)
{
    ms_exa_port_private *port_priv = data;
    ScreenPtr screen = pScrn->pScreen;
    PixmapPtr src_pixmap, dst_pixmap;
    pixman_f_transform_t transform;
    double sx, sy, tx, ty;
    int ret = Success;

    if (id != FOURCC_NV12 && id != DRM_FORMAT_XRGB8888)
        return BadMatch;

    src_pixmap = ms_exa_xv_create_dma_pixmap(pScrn, port_priv, id);
    if (!src_pixmap) {
        src_pixmap = ms_exa_xv_create_pixmap(pScrn, port_priv, id,
                                             buf, width, height);
        if (!src_pixmap)
            return BadMatch;
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
        dst_pixmap = screen->GetWindowPixmap((WindowPtr) pDrawable);
    else
        dst_pixmap = (PixmapPtr) pDrawable;

    DamageRegionAppend(pDrawable, clipBoxes);

    sx = (double)src_w / drw_w;
    sy = (double)src_h / drw_h;

    tx = drw_x - src_x;
    ty = drw_y - src_y;

#ifdef COMPOSITE
    RegionTranslate(clipBoxes, -dst_pixmap->screen_x, -dst_pixmap->screen_y);
    tx -= dst_pixmap->screen_x;
    ty -= dst_pixmap->screen_y;
#endif

    pixman_f_transform_init_scale(&transform, sx, sy);
    pixman_f_transform_translate(NULL, &transform, tx, ty);

    if (!ms_exa_copy_area(src_pixmap, dst_pixmap, &transform, clipBoxes))
        ret = BadMatch;

    DamageRegionProcessPending(pDrawable);

    screen->DestroyPixmap(src_pixmap);

    return ret;
}

static XF86VideoEncodingRec DummyEncoding[1] = {
    { 0, "XV_IMAGE", 8192, 8192, {1, 1} }
};

XF86VideoAdaptorPtr
ms_exa_xv_init(ScreenPtr screen, int num_texture_ports)
{
    ms_exa_port_private *port_priv;
    XF86VideoAdaptorPtr adapt;
    int i;

    msDmaClient = MAKE_ATOM("XV_DMA_CLIENT_ID");
    msDmaHorStride = MAKE_ATOM("XV_DMA_HOR_STRIDE");
    msDmaVerStride = MAKE_ATOM("XV_DMA_VER_STRIDE");
    msDmaDrmFourcc = MAKE_ATOM("XV_DMA_DRM_FOURCC");

    adapt = calloc(1, sizeof(XF86VideoAdaptorRec) + num_texture_ports *
                   (sizeof(ms_exa_port_private) + sizeof(DevUnion)));
    if (adapt == NULL)
        return NULL;

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    adapt->flags = 0;
    adapt->name = "Modesetting Textured Video";
    adapt->nEncodings = 1;
    adapt->pEncodings = DummyEncoding;

    adapt->nFormats = NUM_FORMATS;
    adapt->pFormats = Formats;
    adapt->nPorts = num_texture_ports;
    adapt->pPortPrivates = (DevUnion *) (&adapt[1]);

    adapt->pAttributes = ms_exa_xv_attributes;
    adapt->nAttributes = ms_exa_xv_num_attributes;

    port_priv =
        (ms_exa_port_private *) (&adapt->pPortPrivates[num_texture_ports]);

    adapt->pImages = ms_exa_xv_images;
    adapt->nImages = ms_exa_xv_num_images;

    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = ms_exa_xv_stop_video;
    adapt->SetPortAttribute = ms_exa_xv_set_port_attribute;
    adapt->GetPortAttribute = ms_exa_xv_get_port_attribute;
    adapt->QueryBestSize = ms_exa_xv_query_best_size;
    adapt->PutImage = ms_exa_xv_put_image;
    adapt->ReputImage = NULL;
    adapt->QueryImageAttributes = ms_exa_xv_query_image_attributes;

    for (i = 0; i < num_texture_ports; i++) {
        ms_exa_port_private *priv = &port_priv[i];

        priv->dma_client = 0;
        priv->dma_socket_fd = 0;
        priv->dma_hor_stride = 0;
        priv->dma_ver_stride = 0;
        priv->dma_drm_fourcc = 0;

        adapt->pPortPrivates[i].ptr = (void *) (priv);
    }
    return adapt;
}
