/*
 * Copyright © 2013 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *      Dave Airlie <airlied@redhat.com>
 *
 * some code is derived from the xf86-video-ati radeon driver, mainly
 * the calculations.
 */

/** @file glamor_xv.c
 *
 * Xv acceleration implementation
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glamor_priv.h"
#include "glamor_transform.h"
#include "glamor_transfer.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <libdrm/drm_fourcc.h>

#include <X11/extensions/Xv.h>
#include "../hw/xfree86/common/fourcc.h"
/* Reference color space transform data */
typedef struct tagREF_TRANSFORM {
    float RefLuma;
    float RefRCb;
    float RefRCr;
    float RefGCb;
    float RefGCr;
    float RefBCb;
    float RefBCr;
} REF_TRANSFORM;

#define RTFSaturation(a)   (1.0 + ((a)*1.0)/1000.0)
#define RTFBrightness(a)   (((a)*1.0)/2000.0)
#define RTFIntensity(a)   (((a)*1.0)/2000.0)
#define RTFContrast(a)   (1.0 + ((a)*1.0)/1000.0)
#define RTFHue(a)   (((a)*3.1416)/1000.0)

#ifndef DRM_FORMAT_NV12_10
#define DRM_FORMAT_NV12_10 fourcc_code('N', 'A', '1', '2')
#endif

#ifndef DRM_FORMAT_NV15
#define DRM_FORMAT_NV15 fourcc_code('N', 'V', '1', '5')
#endif

#ifndef DRM_FORMAT_YUV420_8BIT
#define DRM_FORMAT_YUV420_8BIT fourcc_code('Y', 'U', '0', '8')
#endif

#ifndef DRM_FORMAT_YUV420_10BIT
#define DRM_FORMAT_YUV420_10BIT fourcc_code('Y', 'U', '1', '0')
#endif

#ifndef DRM_FORMAT_MOD_VENDOR_ARM
#define DRM_FORMAT_MOD_VENDOR_ARM 0x08
#endif

#ifndef DRM_FORMAT_MOD_ARM_AFBC
#define DRM_FORMAT_MOD_ARM_AFBC(__afbc_mode) fourcc_mod_code(ARM, __afbc_mode)
#endif

#ifndef AFBC_FORMAT_MOD_BLOCK_SIZE_16x16
#define AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 (1ULL)
#endif

#ifndef AFBC_FORMAT_MOD_SPARSE
#define AFBC_FORMAT_MOD_SPARSE (((__u64)1) << 6)
#endif

#define DRM_AFBC_MODIFIER \
  (DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_SPARSE) | \
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16))

#define XV_MAX_DMA_FD 3

static const glamor_facet glamor_facet_xv_planar_2 = {
    .name = "xv_planar_2",

    .version = 120,

    .source_name = "v_texcoord0",
    .vs_vars = ("attribute vec2 position;\n"
                "attribute vec2 v_texcoord0;\n"
                "varying vec2 tcs;\n"),
    .vs_exec = (GLAMOR_POS(gl_Position, position)
                "        tcs = v_texcoord0;\n"),

    .fs_vars = ("uniform sampler2D y_sampler;\n"
                "uniform sampler2D u_sampler;\n"
                "uniform vec4 offsetyco;\n"
                "uniform vec4 ucogamma;\n"
                "uniform vec4 vco;\n"
                "varying vec2 tcs;\n"),
    .fs_exec = (
                "        float sample;\n"
                "        vec2 sample_uv;\n"
                "        vec4 temp1;\n"
                "        sample = texture2D(y_sampler, tcs).w;\n"
                "        temp1.xyz = offsetyco.www * vec3(sample) + offsetyco.xyz;\n"
                "        sample_uv = texture2D(u_sampler, tcs).xy;\n"
                "        temp1.xyz = ucogamma.xyz * vec3(sample_uv.x) + temp1.xyz;\n"
                "        temp1.xyz = clamp(vco.xyz * vec3(sample_uv.y) + temp1.xyz, 0.0, 1.0);\n"
                "        temp1.w = 1.0;\n"
                "        gl_FragColor = temp1;\n"
                ),
};

static const glamor_facet glamor_facet_xv_planar_3 = {
    .name = "xv_planar_3",

    .version = 120,

    .source_name = "v_texcoord0",
    .vs_vars = ("attribute vec2 position;\n"
                "attribute vec2 v_texcoord0;\n"
                "varying vec2 tcs;\n"),
    .vs_exec = (GLAMOR_POS(gl_Position, position)
                "        tcs = v_texcoord0;\n"),

    .fs_vars = ("uniform sampler2D y_sampler;\n"
                "uniform sampler2D u_sampler;\n"
                "uniform sampler2D v_sampler;\n"
                "uniform vec4 offsetyco;\n"
                "uniform vec4 ucogamma;\n"
                "uniform vec4 vco;\n"
                "varying vec2 tcs;\n"),
    .fs_exec = (
                "        float sample;\n"
                "        vec4 temp1;\n"
                "        sample = texture2D(y_sampler, tcs).w;\n"
                "        temp1.xyz = offsetyco.www * vec3(sample) + offsetyco.xyz;\n"
                "        sample = texture2D(u_sampler, tcs).w;\n"
                "        temp1.xyz = ucogamma.xyz * vec3(sample) + temp1.xyz;\n"
                "        sample = texture2D(v_sampler, tcs).w;\n"
                "        temp1.xyz = clamp(vco.xyz * vec3(sample) + temp1.xyz, 0.0, 1.0);\n"
                "        temp1.w = 1.0;\n"
                "        gl_FragColor = temp1;\n"
                ),
};

static const glamor_facet glamor_facet_xv_egl_external = {
    .name = "xv_egl_external",

    .source_name = "v_texcoord0",
    .vs_vars = ("attribute vec2 position;\n"
                "attribute vec2 v_texcoord0;\n"
                "varying vec2 tcs;\n"),
    .vs_exec = (GLAMOR_POS(gl_Position, position)
                "        tcs = v_texcoord0;\n"),

    .fs_extensions = ("#extension GL_OES_EGL_image_external : require\n"),
    .fs_vars = ("uniform samplerExternalOES sampler;\n"
                "varying vec2 tcs;\n"),
    .fs_exec = (
                "        gl_FragColor = texture2D(sampler, tcs);\n"
                ),
};

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

XvAttributeRec glamor_xv_attributes[] = {
    {XvSettable | XvGettable, -1000, 1000, (char *)"XV_BRIGHTNESS"},
    {XvSettable | XvGettable, -1000, 1000, (char *)"XV_CONTRAST"},
    {XvSettable | XvGettable, -1000, 1000, (char *)"XV_SATURATION"},
    {XvSettable | XvGettable, -1000, 1000, (char *)"XV_HUE"},
    {XvSettable | XvGettable, 0, 1, (char *)"XV_COLORSPACE"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_CLIENT_ID"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_HOR_STRIDE"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_VER_STRIDE"},
    {XvSettable | XvGettable, 0, 0xFFFFFFFF, (char *)"XV_DMA_DRM_FOURCC"},
    {XvSettable | XvGettable, 0, 1, (char *)"XV_DMA_DRM_AFBC"},
    {0, 0, 0, NULL}
};
int glamor_xv_num_attributes = ARRAY_SIZE(glamor_xv_attributes) - 1;

Atom glamorBrightness, glamorContrast, glamorSaturation, glamorHue,
    glamorColorspace, glamorGamma, glamorDmaClient, glamorDmaHorStride,
    glamorDmaVerStride, glamorDmaDrmFourcc, glamorDmaDrmAFBC;

XvImageRec glamor_xv_images[] = {
    XVIMAGE_YV12,
    XVIMAGE_I420,
    XVIMAGE_NV12
};
int glamor_xv_num_images = ARRAY_SIZE(glamor_xv_images);

static void
glamor_init_xv_shader(ScreenPtr screen, int id)
{
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);
    GLint sampler_loc;
    const glamor_facet *glamor_facet_xv_planar = NULL;

    switch (id) {
    case FOURCC_YV12:
    case FOURCC_I420:
        glamor_facet_xv_planar = &glamor_facet_xv_planar_3;
        break;
    case FOURCC_NV12:
        glamor_facet_xv_planar = &glamor_facet_xv_planar_2;
        break;
    default:
        break;
    }

    glamor_build_program(screen,
                         &glamor_priv->xv_prog,
                         glamor_facet_xv_planar, NULL, NULL, NULL);

    glUseProgram(glamor_priv->xv_prog.prog);
    sampler_loc = glGetUniformLocation(glamor_priv->xv_prog.prog, "y_sampler");
    glUniform1i(sampler_loc, 0);
    sampler_loc = glGetUniformLocation(glamor_priv->xv_prog.prog, "u_sampler");
    glUniform1i(sampler_loc, 1);

    switch (id) {
    case FOURCC_YV12:
    case FOURCC_I420:
        sampler_loc = glGetUniformLocation(glamor_priv->xv_prog.prog, "v_sampler");
        glUniform1i(sampler_loc, 2);
        break;
    case FOURCC_NV12:
        break;
    default:
        break;
    }

}

static void
glamor_init_xv_shader_egl_external(ScreenPtr screen)
{
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);
    GLint sampler_loc;

    glamor_build_program(screen,
                         &glamor_priv->xv_prog_ext,
                         &glamor_facet_xv_egl_external, NULL, NULL, NULL);
    glUseProgram(glamor_priv->xv_prog_ext.prog);

    sampler_loc =
        glGetUniformLocation(glamor_priv->xv_prog_ext.prog, "sampler");
    glUniform1i(sampler_loc, 0);
}

#define ClipValue(v,min,max) ((v) < (min) ? (min) : (v) > (max) ? (max) : (v))

static void
glamor_xv_set_dma_client(glamor_port_private *port_priv,
                         uint32_t dma_client)
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
    port_priv->dma_drm_afbc = 0;
}

void
glamor_xv_stop_video(glamor_port_private *port_priv)
{
    glamor_xv_set_dma_client(port_priv, 0);
}

static void
glamor_xv_free_port_data(glamor_port_private *port_priv)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (port_priv->src_pix[i]) {
            glamor_destroy_pixmap(port_priv->src_pix[i]);
            port_priv->src_pix[i] = NULL;
        }
    }
    RegionUninit(&port_priv->clip);
    RegionNull(&port_priv->clip);
}

int
glamor_xv_set_port_attribute(glamor_port_private *port_priv,
                             Atom attribute, INT32 value)
{
    if (attribute == glamorBrightness)
        port_priv->brightness = ClipValue(value, -1000, 1000);
    else if (attribute == glamorHue)
        port_priv->hue = ClipValue(value, -1000, 1000);
    else if (attribute == glamorContrast)
        port_priv->contrast = ClipValue(value, -1000, 1000);
    else if (attribute == glamorSaturation)
        port_priv->saturation = ClipValue(value, -1000, 1000);
    else if (attribute == glamorGamma)
        port_priv->gamma = ClipValue(value, 100, 10000);
    else if (attribute == glamorColorspace)
        port_priv->transform_index = ClipValue(value, 0, 1);
    else if (attribute == glamorDmaClient)
        glamor_xv_set_dma_client(port_priv, ClipValue(value, 0, 0xFFFFFFFF));
    else if (attribute == glamorDmaHorStride)
        port_priv->dma_hor_stride = ClipValue(value, 0, 0xFFFFFFFF);
    else if (attribute == glamorDmaVerStride)
        port_priv->dma_ver_stride = ClipValue(value, 0, 0xFFFFFFFF);
    else if (attribute == glamorDmaDrmFourcc)
        port_priv->dma_drm_fourcc = ClipValue(value, 0, 0xFFFFFFFF);
    else if (attribute == glamorDmaDrmAFBC)
        port_priv->dma_drm_afbc = ClipValue(value, 0, 0xFFFFFFFF);
    else
        return BadMatch;
    return Success;
}

int
glamor_xv_get_port_attribute(glamor_port_private *port_priv,
                             Atom attribute, INT32 *value)
{
    if (attribute == glamorBrightness)
        *value = port_priv->brightness;
    else if (attribute == glamorHue)
        *value = port_priv->hue;
    else if (attribute == glamorContrast)
        *value = port_priv->contrast;
    else if (attribute == glamorSaturation)
        *value = port_priv->saturation;
    else if (attribute == glamorGamma)
        *value = port_priv->gamma;
    else if (attribute == glamorColorspace)
        *value = port_priv->transform_index;
    else if (attribute == glamorDmaClient)
        *value = port_priv->dma_client;
    else if (attribute == glamorDmaHorStride)
        *value = port_priv->dma_hor_stride;
    else if (attribute == glamorDmaVerStride)
        *value = port_priv->dma_ver_stride;
    else if (attribute == glamorDmaDrmFourcc)
        *value = port_priv->dma_drm_fourcc;
    else if (attribute == glamorDmaDrmAFBC)
        *value = port_priv->dma_drm_afbc;
    else
        return BadMatch;

    return Success;
}

int
glamor_xv_query_image_attributes(int id,
                                 unsigned short *w, unsigned short *h,
                                 int *pitches, int *offsets)
{
    int size = 0, tmp;

    if (offsets)
        offsets[0] = 0;
    switch (id) {
    case FOURCC_YV12:
    case FOURCC_I420:
        *w = ALIGN(*w, 2);
        *h = ALIGN(*h, 2);
        size = ALIGN(*w, 4);
        if (pitches)
            pitches[0] = size;
        size *= *h;
        if (offsets)
            offsets[1] = size;
        tmp = ALIGN(*w >> 1, 4);
        if (pitches)
            pitches[1] = pitches[2] = tmp;
        tmp *= (*h >> 1);
        size += tmp;
        if (offsets)
            offsets[2] = size;
        size += tmp;
        break;
    case FOURCC_NV12:
        *w = ALIGN(*w, 2);
        *h = ALIGN(*h, 2);
        size = ALIGN(*w, 4);
        if (pitches)
            pitches[0] = size;
        size *= *h;
        if (offsets)
            offsets[1] = size;
        tmp = ALIGN(*w, 4);
        if (pitches)
            pitches[1] = tmp;
        tmp *= (*h >> 1);
        size += tmp;
        break;
    }
    return size;
}

/* Parameters for ITU-R BT.601 and ITU-R BT.709 colour spaces
   note the difference to the parameters used in overlay are due
   to 10bit vs. float calcs */
static REF_TRANSFORM trans[2] = {
    {1.1643, 0.0, 1.5960, -0.3918, -0.8129, 2.0172, 0.0},       /* BT.601 */
    {1.1643, 0.0, 1.7927, -0.2132, -0.5329, 2.1124, 0.0}        /* BT.709 */
};

void
glamor_xv_render(glamor_port_private *port_priv, int id)
{
    ScreenPtr screen = port_priv->pPixmap->drawable.pScreen;
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);
    PixmapPtr pixmap = port_priv->pPixmap;
    glamor_pixmap_private *pixmap_priv = glamor_get_pixmap_private(pixmap);
    glamor_pixmap_private *src_pixmap_priv[3];
    BoxPtr box = REGION_RECTS(&port_priv->clip);
    int nBox = REGION_NUM_RECTS(&port_priv->clip);
    GLfloat src_xscale[3], src_yscale[3];
    int i;
    const float Loff = -0.0627;
    const float Coff = -0.502;
    float uvcosf, uvsinf;
    float yco;
    float uco[3], vco[3], off[3];
    float bright, cont, gamma;
    int ref = port_priv->transform_index;
    GLint uloc;
    GLfloat *v;
    char *vbo_offset;
    int dst_box_index;

    DamageRegionAppend(port_priv->pDraw, &port_priv->clip);

    if (!glamor_priv->xv_prog.prog)
        glamor_init_xv_shader(screen, id);

    cont = RTFContrast(port_priv->contrast);
    bright = RTFBrightness(port_priv->brightness);
    gamma = (float) port_priv->gamma / 1000.0;
    uvcosf = RTFSaturation(port_priv->saturation) * cos(RTFHue(port_priv->hue));
    uvsinf = RTFSaturation(port_priv->saturation) * sin(RTFHue(port_priv->hue));
/* overlay video also does pre-gamma contrast/sat adjust, should we? */

    yco = trans[ref].RefLuma * cont;
    uco[0] = -trans[ref].RefRCr * uvsinf;
    uco[1] = trans[ref].RefGCb * uvcosf - trans[ref].RefGCr * uvsinf;
    uco[2] = trans[ref].RefBCb * uvcosf;
    vco[0] = trans[ref].RefRCr * uvcosf;
    vco[1] = trans[ref].RefGCb * uvsinf + trans[ref].RefGCr * uvcosf;
    vco[2] = trans[ref].RefBCb * uvsinf;
    off[0] = Loff * yco + Coff * (uco[0] + vco[0]) + bright;
    off[1] = Loff * yco + Coff * (uco[1] + vco[1]) + bright;
    off[2] = Loff * yco + Coff * (uco[2] + vco[2]) + bright;
    gamma = 1.0;

    glamor_set_alu(screen, GXcopy);

    for (i = 0; i < 3; i++) {
        if (port_priv->src_pix[i]) {
            src_pixmap_priv[i] =
                glamor_get_pixmap_private(port_priv->src_pix[i]);
            pixmap_priv_get_scale(src_pixmap_priv[i], &src_xscale[i],
                                  &src_yscale[i]);
        } else {
           src_pixmap_priv[i] = NULL;
        }
    }
    glamor_make_current(glamor_priv);
    glUseProgram(glamor_priv->xv_prog.prog);

    uloc = glGetUniformLocation(glamor_priv->xv_prog.prog, "offsetyco");
    glUniform4f(uloc, off[0], off[1], off[2], yco);
    uloc = glGetUniformLocation(glamor_priv->xv_prog.prog, "ucogamma");
    glUniform4f(uloc, uco[0], uco[1], uco[2], gamma);
    uloc = glGetUniformLocation(glamor_priv->xv_prog.prog, "vco");
    glUniform4f(uloc, vco[0], vco[1], vco[2], 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_pixmap_priv[0]->fbo->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, src_pixmap_priv[1]->fbo->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    switch (id) {
    case FOURCC_YV12:
    case FOURCC_I420:
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, src_pixmap_priv[2]->fbo->tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case FOURCC_NV12:
        break;
    default:
        break;
    }

    glEnableVertexAttribArray(GLAMOR_VERTEX_POS);
    glEnableVertexAttribArray(GLAMOR_VERTEX_SOURCE);

    glEnable(GL_SCISSOR_TEST);

    v = glamor_get_vbo_space(screen, 3 * 4 * sizeof(GLfloat), &vbo_offset);

    /* Set up a single primitive covering the area being drawn.  We'll
     * clip it to port_priv->clip using GL scissors instead of just
     * emitting a GL_QUAD per box, because this way we hopefully avoid
     * diagonal tearing between the two triangles used to rasterize a
     * GL_QUAD.
     */
    i = 0;
    v[i++] = port_priv->drw_x;
    v[i++] = port_priv->drw_y;

    v[i++] = port_priv->drw_x + port_priv->dst_w * 2;
    v[i++] = port_priv->drw_y;

    v[i++] = port_priv->drw_x;
    v[i++] = port_priv->drw_y + port_priv->dst_h * 2;

    v[i++] = t_from_x_coord_x(src_xscale[0], port_priv->src_x);
    v[i++] = t_from_x_coord_y(src_yscale[0], port_priv->src_y);

    v[i++] = t_from_x_coord_x(src_xscale[0], port_priv->src_x +
                              port_priv->src_w * 2);
    v[i++] = t_from_x_coord_y(src_yscale[0], port_priv->src_y);

    v[i++] = t_from_x_coord_x(src_xscale[0], port_priv->src_x);
    v[i++] = t_from_x_coord_y(src_yscale[0], port_priv->src_y +
                              port_priv->src_h * 2);

    glVertexAttribPointer(GLAMOR_VERTEX_POS, 2,
                          GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), vbo_offset);

    glVertexAttribPointer(GLAMOR_VERTEX_SOURCE, 2,
                          GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), vbo_offset + 6 * sizeof(GLfloat));

    glamor_put_vbo_space(screen);

    /* Now draw our big triangle, clipped to each of the clip boxes. */
    glamor_pixmap_loop(pixmap_priv, dst_box_index) {
        int dst_off_x, dst_off_y;

        glamor_set_destination_drawable(port_priv->pDraw,
                                        dst_box_index,
                                        FALSE, FALSE,
                                        glamor_priv->xv_prog.matrix_uniform,
                                        &dst_off_x, &dst_off_y);

        for (i = 0; i < nBox; i++) {
            int dstx, dsty, dstw, dsth;

            dstx = box[i].x1 + dst_off_x;
            dsty = box[i].y1 + dst_off_y;
            dstw = box[i].x2 - box[i].x1;
            dsth = box[i].y2 - box[i].y1;

            glScissor(dstx, dsty, dstw, dsth);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        }
    }
    glDisable(GL_SCISSOR_TEST);

    glDisableVertexAttribArray(GLAMOR_VERTEX_POS);
    glDisableVertexAttribArray(GLAMOR_VERTEX_SOURCE);

    DamageRegionProcessPending(port_priv->pDraw);

    glamor_xv_free_port_data(port_priv);

    glamor_pixmap_invalid(pixmap);
}

static int
glamor_xv_render_dma(glamor_port_private *port_priv, int dma_fd)
{
    ScreenPtr screen = port_priv->pPixmap->drawable.pScreen;
    glamor_screen_private *glamor_priv = glamor_get_screen_private(screen);
    PixmapPtr pixmap = port_priv->pPixmap;
    glamor_pixmap_private *pixmap_priv = glamor_get_pixmap_private(pixmap);
    BoxPtr box = REGION_RECTS(&port_priv->clip);
    int nBox = REGION_NUM_RECTS(&port_priv->clip);
    GLfloat src_xscale, src_yscale;
    int i;
    GLfloat *v;
    char *vbo_offset;
    int dst_box_index;

    int hor_stride =
        port_priv->dma_hor_stride ? port_priv->dma_hor_stride : port_priv->w;
    int ver_stride =
        port_priv->dma_ver_stride ? port_priv->dma_ver_stride : port_priv->h;
    int width = hor_stride;
    uint32_t fourcc =
        port_priv->dma_drm_fourcc ? port_priv->dma_drm_fourcc : DRM_FORMAT_NV12;
    int afbc = port_priv->dma_drm_afbc;

    PFNEGLCREATEIMAGEKHRPROC create_image;
    PFNEGLDESTROYIMAGEKHRPROC destroy_image;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d;
    EGLImageKHR image;
    GLuint texture;

    /* Mali is using NV15 for NV12_10 */
    switch (fourcc) {
    case DRM_FORMAT_NV12_10:
    case DRM_FORMAT_NV15:
        fourcc = DRM_FORMAT_NV15;

        /* HACK: guess a width from 10B stride */
        width = hor_stride / 10 * 8;

        if (afbc) {
            fourcc = DRM_FORMAT_YUV420_10BIT;
            hor_stride *= 1.5;
        }
        break;
    case DRM_FORMAT_NV12:
        if (afbc) {
            fourcc = DRM_FORMAT_YUV420_8BIT;
            hor_stride *= 1.5;
        }
        break;
    case DRM_FORMAT_NV16:
        if (afbc) {
            fourcc = DRM_FORMAT_YUYV;
            hor_stride *= 2;
        }
        break;
    default:
        ErrorF("glamor xv only support DMA for NV12|NV12_10|NV16\n");
        return BadMatch;
    }

    const EGLint attrs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, ver_stride,
        EGL_LINUX_DRM_FOURCC_EXT, fourcc,
        EGL_DMA_BUF_PLANE0_FD_EXT, dma_fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, hor_stride,
        EGL_DMA_BUF_PLANE1_FD_EXT, dma_fd,
        EGL_DMA_BUF_PLANE1_OFFSET_EXT, hor_stride * ver_stride,
        EGL_DMA_BUF_PLANE1_PITCH_EXT, hor_stride,
        EGL_YUV_COLOR_SPACE_HINT_EXT, EGL_ITU_REC601_EXT,
        EGL_SAMPLE_RANGE_HINT_EXT, EGL_YUV_NARROW_RANGE_EXT,
        EGL_NONE
    };

    const EGLint attrs_afbc[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, ver_stride,
        EGL_LINUX_DRM_FOURCC_EXT, fourcc,
        EGL_DMA_BUF_PLANE0_FD_EXT, dma_fd,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, hor_stride,
        EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT, DRM_AFBC_MODIFIER & 0xFFFFFFFF,
        EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT, DRM_AFBC_MODIFIER >> 32,
        EGL_NONE
    };

    create_image =
        (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    destroy_image =
        (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
    image_target_texture_2d =
        (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
        eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!create_image || !destroy_image || !image_target_texture_2d) {
        ErrorF("glamor xv without EGL_EXT_image_dma_buf_import\n");
        return BadMatch;
    }

    glamor_make_current(glamor_priv);

    image = create_image(glamor_priv->ctx.display, EGL_NO_CONTEXT,
                         EGL_LINUX_DMA_BUF_EXT, NULL,
                         afbc ? attrs_afbc : attrs);
    if (image == EGL_NO_IMAGE) {
        ErrorF("glamor xv failed to create egl image\n");
        return BadMatch;
    }

    DamageRegionAppend(port_priv->pDraw, &port_priv->clip);

    if (!glamor_priv->xv_prog_ext.prog)
        glamor_init_xv_shader_egl_external(screen);

    /* TODO: support contrast/brightness/gamma/saturation/hue */
    glamor_set_alu(screen, GXcopy);

    src_xscale = 1.0 / width;
    src_yscale = 1.0 / ver_stride;

    glUseProgram(glamor_priv->xv_prog_ext.prog);

    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
    image_target_texture_2d(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)image);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnableVertexAttribArray(GLAMOR_VERTEX_POS);
    glEnableVertexAttribArray(GLAMOR_VERTEX_SOURCE);

    glEnable(GL_SCISSOR_TEST);

    v = glamor_get_vbo_space(screen, 3 * 4 * sizeof(GLfloat), &vbo_offset);

    /* Set up a single primitive covering the area being drawn.  We'll
     * clip it to port_priv->clip using GL scissors instead of just
     * emitting a GL_QUAD per box, because this way we hopefully avoid
     * diagonal tearing between the two triangles used to rasterize a
     * GL_QUAD.
     */
    i = 0;
    v[i++] = port_priv->drw_x;
    v[i++] = port_priv->drw_y;

    v[i++] = port_priv->drw_x + port_priv->dst_w * 2;
    v[i++] = port_priv->drw_y;

    v[i++] = port_priv->drw_x;
    v[i++] = port_priv->drw_y + port_priv->dst_h * 2;

    v[i++] = t_from_x_coord_x(src_xscale, port_priv->src_x);
    v[i++] = t_from_x_coord_y(src_yscale, port_priv->src_y);

    v[i++] = t_from_x_coord_x(src_xscale, port_priv->src_x +
                              port_priv->src_w * 2);
    v[i++] = t_from_x_coord_y(src_yscale, port_priv->src_y);

    v[i++] = t_from_x_coord_x(src_xscale, port_priv->src_x);
    v[i++] = t_from_x_coord_y(src_yscale, port_priv->src_y +
                              port_priv->src_h * 2);

    glVertexAttribPointer(GLAMOR_VERTEX_POS, 2,
                          GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), vbo_offset);

    glVertexAttribPointer(GLAMOR_VERTEX_SOURCE, 2,
                          GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), vbo_offset + 6 * sizeof(GLfloat));

    glamor_put_vbo_space(screen);

    /* Now draw our big triangle, clipped to each of the clip boxes. */
    glamor_pixmap_loop(pixmap_priv, dst_box_index) {
        int dst_off_x, dst_off_y;

        glamor_set_destination_drawable(port_priv->pDraw,
                                        dst_box_index,
                                        FALSE, FALSE,
                                        glamor_priv->xv_prog_ext.matrix_uniform,
                                        &dst_off_x, &dst_off_y);

        for (i = 0; i < nBox; i++) {
            int dstx, dsty, dstw, dsth;

            dstx = box[i].x1 + dst_off_x;
            dsty = box[i].y1 + dst_off_y;
            dstw = box[i].x2 - box[i].x1;
            dsth = box[i].y2 - box[i].y1;

            glScissor(dstx, dsty, dstw, dsth);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        }
    }
    glDisable(GL_SCISSOR_TEST);

    glDisableVertexAttribArray(GLAMOR_VERTEX_POS);
    glDisableVertexAttribArray(GLAMOR_VERTEX_SOURCE);

    DamageRegionProcessPending(port_priv->pDraw);

    glamor_xv_free_port_data(port_priv);

    glDeleteTextures(1, &texture);
    destroy_image(glamor_priv->ctx.display, image);

    glamor_pixmap_invalid(pixmap);

    return Success;
}

static int
glamor_xv_put_dma_image(glamor_port_private *port_priv,
                        DrawablePtr pDrawable,
                        short src_x, short src_y,
                        short drw_x, short drw_y,
                        short src_w, short src_h,
                        short drw_w, short drw_h,
                        int id,
                        short width,
                        short height,
                        Bool sync,
                        RegionPtr clipBoxes)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *header;
    char buf[CMSG_SPACE (sizeof (int))];
    int dma_fds[XV_MAX_DMA_FD], num_dma_fd;
    int ret = BadMatch;

    if (!port_priv->dma_client || port_priv->dma_socket_fd <= 0)
        return BadMatch;

    /* Only support NV12 for now */
    if (id != FOURCC_NV12)
        return BadValue;

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

    /* Expected 1 buffer for NV12 */
    if (num_dma_fd != 1)
        goto out;

    if (pDrawable->type == DRAWABLE_WINDOW)
        port_priv->pPixmap = pScreen->GetWindowPixmap((WindowPtr) pDrawable);
    else
        port_priv->pPixmap = (PixmapPtr) pDrawable;

    RegionCopy(&port_priv->clip, clipBoxes);

    port_priv->src_x = src_x;
    port_priv->src_y = src_y;
    port_priv->src_w = src_w;
    port_priv->src_h = src_h;
    port_priv->dst_w = drw_w;
    port_priv->dst_h = drw_h;
    port_priv->drw_x = drw_x;
    port_priv->drw_y = drw_y;
    port_priv->w = width;
    port_priv->h = height;
    port_priv->pDraw = pDrawable;

    ret = glamor_xv_render_dma(port_priv, dma_fds[0]);
    if (ret == Success && sync)
        glamor_finish(pScreen);

out:
    while (num_dma_fd--)
        close(dma_fds[num_dma_fd]);

    if (ret != Success) {
        ErrorF("glamor xv failed to render dma image\n");
        glamor_xv_set_dma_client(port_priv, 0);
    }

    return ret;
}

int
glamor_xv_put_image(glamor_port_private *port_priv,
                    DrawablePtr pDrawable,
                    short src_x, short src_y,
                    short drw_x, short drw_y,
                    short src_w, short src_h,
                    short drw_w, short drw_h,
                    int id,
                    unsigned char *buf,
                    short width,
                    short height,
                    Bool sync,
                    RegionPtr clipBoxes)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    glamor_screen_private *glamor_priv = glamor_get_screen_private(pScreen);
    int srcPitch, srcPitch2;
    int top, nlines;
    int s2offset, s3offset, tmp;
    BoxRec full_box, half_box;

    if (glamor_xv_put_dma_image(port_priv, pDrawable,
                                src_x, src_y, drw_x, drw_y,
                                src_w, src_h, drw_w, drw_h,
                                id, width, height, sync, clipBoxes) == Success)
        return Success;

    s2offset = s3offset = srcPitch2 = 0;

    if (!port_priv->src_pix[0] ||
        (width != port_priv->src_pix_w || height != port_priv->src_pix_h) ||
        (port_priv->src_pix[2] && id == FOURCC_NV12) ||
        (!port_priv->src_pix[2] && id != FOURCC_NV12)) {
        int i;

        if (glamor_priv->xv_prog.prog) {
            glDeleteProgram(glamor_priv->xv_prog.prog);
            glamor_priv->xv_prog.prog = 0;
        }

        for (i = 0; i < 3; i++)
            if (port_priv->src_pix[i])
                glamor_destroy_pixmap(port_priv->src_pix[i]);

        port_priv->src_pix[0] =
            glamor_create_pixmap(pScreen, width, height, 8,
                                 GLAMOR_CREATE_FBO_NO_FBO);

        switch (id) {
        case FOURCC_YV12:
        case FOURCC_I420:
            port_priv->src_pix[1] =
                glamor_create_pixmap(pScreen, width >> 1, height >> 1, 8,
                                     GLAMOR_CREATE_FBO_NO_FBO);
            port_priv->src_pix[2] =
                glamor_create_pixmap(pScreen, width >> 1, height >> 1, 8,
                                     GLAMOR_CREATE_FBO_NO_FBO);
            if (!port_priv->src_pix[2])
                return BadAlloc;
            break;
        case FOURCC_NV12:
            port_priv->src_pix[1] =
                glamor_create_pixmap(pScreen, width >> 1, height >> 1, 16,
                                     GLAMOR_CREATE_FBO_NO_FBO |
                                     GLAMOR_CREATE_FORMAT_CBCR);
            port_priv->src_pix[2] = NULL;
            break;
        default:
            return BadMatch;
        }

        port_priv->src_pix_w = width;
        port_priv->src_pix_h = height;

        if (!port_priv->src_pix[0] || !port_priv->src_pix[1])
            return BadAlloc;
    }

    top = (src_y) & ~1;
    nlines = (src_y + src_h) - top;

    switch (id) {
    case FOURCC_YV12:
    case FOURCC_I420:
        srcPitch = ALIGN(width, 4);
        srcPitch2 = ALIGN(width >> 1, 4);
        s2offset = srcPitch * height;
        s3offset = s2offset + (srcPitch2 * ((height + 1) >> 1));
        s2offset += ((top >> 1) * srcPitch2);
        s3offset += ((top >> 1) * srcPitch2);
        if (id == FOURCC_YV12) {
            tmp = s2offset;
            s2offset = s3offset;
            s3offset = tmp;
        }

        full_box.x1 = 0;
        full_box.y1 = 0;
        full_box.x2 = width;
        full_box.y2 = nlines;

        half_box.x1 = 0;
        half_box.y1 = 0;
        half_box.x2 = width >> 1;
        half_box.y2 = (nlines + 1) >> 1;

        glamor_upload_boxes(port_priv->src_pix[0], &full_box, 1,
                            0, 0, 0, 0,
                            buf + (top * srcPitch), srcPitch);

        glamor_upload_boxes(port_priv->src_pix[1], &half_box, 1,
                            0, 0, 0, 0,
                            buf + s2offset, srcPitch2);

        glamor_upload_boxes(port_priv->src_pix[2], &half_box, 1,
                            0, 0, 0, 0,
                            buf + s3offset, srcPitch2);
        break;
    case FOURCC_NV12:
        srcPitch = ALIGN(width, 4);
        s2offset = srcPitch * height;
        s2offset += ((top >> 1) * srcPitch);

        full_box.x1 = 0;
        full_box.y1 = 0;
        full_box.x2 = width;
        full_box.y2 = nlines;

        half_box.x1 = 0;
        half_box.y1 = 0;
        half_box.x2 = width;
        half_box.y2 = (nlines + 1) >> 1;

        glamor_upload_boxes(port_priv->src_pix[0], &full_box, 1,
                            0, 0, 0, 0,
                            buf + (top * srcPitch), srcPitch);

        glamor_upload_boxes(port_priv->src_pix[1], &half_box, 1,
                            0, 0, 0, 0,
                            buf + s2offset, srcPitch);
        break;
    default:
        return BadMatch;
    }

    if (pDrawable->type == DRAWABLE_WINDOW)
        port_priv->pPixmap = pScreen->GetWindowPixmap((WindowPtr) pDrawable);
    else
        port_priv->pPixmap = (PixmapPtr) pDrawable;

    RegionCopy(&port_priv->clip, clipBoxes);

    port_priv->src_x = src_x;
    port_priv->src_y = src_y - top;
    port_priv->src_w = src_w;
    port_priv->src_h = src_h;
    port_priv->dst_w = drw_w;
    port_priv->dst_h = drw_h;
    port_priv->drw_x = drw_x;
    port_priv->drw_y = drw_y;
    port_priv->w = width;
    port_priv->h = height;
    port_priv->pDraw = pDrawable;
    glamor_xv_render(port_priv, id);
    return Success;
}

void
glamor_xv_init_port(glamor_port_private *port_priv)
{
    port_priv->brightness = 0;
    port_priv->contrast = 0;
    port_priv->saturation = 0;
    port_priv->hue = 0;
    port_priv->gamma = 1000;
    port_priv->transform_index = 0;
    port_priv->dma_client = 0;
    port_priv->dma_socket_fd = 0;
    port_priv->dma_hor_stride = 0;
    port_priv->dma_ver_stride = 0;
    port_priv->dma_drm_fourcc = 0;
    port_priv->dma_drm_afbc = 0;

    REGION_NULL(pScreen, &port_priv->clip);
}

void
glamor_xv_core_init(ScreenPtr screen)
{
    glamorBrightness = MAKE_ATOM("XV_BRIGHTNESS");
    glamorContrast = MAKE_ATOM("XV_CONTRAST");
    glamorSaturation = MAKE_ATOM("XV_SATURATION");
    glamorHue = MAKE_ATOM("XV_HUE");
    glamorGamma = MAKE_ATOM("XV_GAMMA");
    glamorColorspace = MAKE_ATOM("XV_COLORSPACE");
    glamorDmaClient = MAKE_ATOM("XV_DMA_CLIENT_ID");
    glamorDmaHorStride = MAKE_ATOM("XV_DMA_HOR_STRIDE");
    glamorDmaVerStride = MAKE_ATOM("XV_DMA_VER_STRIDE");
    glamorDmaDrmFourcc = MAKE_ATOM("XV_DMA_DRM_FOURCC");
    glamorDmaDrmAFBC = MAKE_ATOM("XV_DMA_DRM_AFBC");
}
