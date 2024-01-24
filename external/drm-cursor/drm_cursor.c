/*
 *  Copyright (c) 2021, Jeffy Chen <jeffy.chen@rock-chips.com>
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

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <gbm.h>

#include "drm_common.h"
#include "drm_egl.h"

#define DRM_CURSOR_CONFIG_FILE "/etc/drm-cursor.conf"
#define OPT_DEBUG "debug="
#define OPT_LOG_FILE "log-file="
#define OPT_HIDE "hide="
#define OPT_ALLOW_OVERLAY "allow-overlay="
#define OPT_PREFER_AFBC "prefer-afbc="
#define OPT_PREFER_PLANE "prefer-plane="
#define OPT_PREFER_PLANES "prefer-planes="
#define OPT_CRTC_BLOCKLIST "crtc-blocklist="
#define OPT_NUM_SURFACES "num-surfaces="
#define OPT_MAX_FPS "max-fps="
#define OPT_ATOMIC "atomic="
#define OPT_SCALE "scale="
#define OPT_SCALE_FROM "scale-from="

#define DRM_MAX_CRTCS 8

typedef enum {
  PLANE_PROP_type = 0,
  PLANE_PROP_IN_FORMATS,
  PLANE_PROP_zpos,
  PLANE_PROP_ZPOS,
  PLANE_PROP_ASYNC_COMMIT,
  PLANE_PROP_CRTC_ID,
  PLANE_PROP_FB_ID,
  PLANE_PROP_SRC_X,
  PLANE_PROP_SRC_Y,
  PLANE_PROP_SRC_W,
  PLANE_PROP_SRC_H,
  PLANE_PROP_CRTC_X,
  PLANE_PROP_CRTC_Y,
  PLANE_PROP_CRTC_W,
  PLANE_PROP_CRTC_H,
  PLANE_PROP_MAX,
} drm_plane_prop;

static const char *drm_plane_prop_names[] = {
  [PLANE_PROP_type] = "type",
  [PLANE_PROP_IN_FORMATS] = "IN_FORMATS",
  [PLANE_PROP_zpos] = "zpos",
  [PLANE_PROP_ZPOS] = "ZPOS",
  [PLANE_PROP_ASYNC_COMMIT] = "ASYNC_COMMIT",
  [PLANE_PROP_CRTC_ID] = "CRTC_ID",
  [PLANE_PROP_FB_ID] = "FB_ID",
  [PLANE_PROP_SRC_X] = "SRC_X",
  [PLANE_PROP_SRC_Y] = "SRC_Y",
  [PLANE_PROP_SRC_W] = "SRC_W",
  [PLANE_PROP_SRC_H] = "SRC_H",
  [PLANE_PROP_CRTC_X] = "CRTC_X",
  [PLANE_PROP_CRTC_Y] = "CRTC_Y",
  [PLANE_PROP_CRTC_W] = "CRTC_W",
  [PLANE_PROP_CRTC_H] = "CRTC_H",
};

typedef struct {
  uint32_t plane_id;
  int cursor_plane;
  int can_afbc;
  int can_linear;
  drmModePlane *plane;
  drmModeObjectProperties *props;
  int prop_ids[PLANE_PROP_MAX];
} drm_plane;

#define REQ_SET_CURSOR  (1 << 0)
#define REQ_MOVE_CURSOR (1 << 1)

typedef struct {
  uint32_t handle;
  uint32_t fb;

  int width;
  int height;

  int scaled_w;
  int scaled_h;

  int x;
  int y;

  int scaled_x;
  int scaled_y;

  int off_x;
  int off_y;

  int hot_x;
  int hot_y;

  int request;
} drm_cursor_state;

typedef enum {
  IDLE = 0,
  FATAL_ERROR,
  PENDING,
} drm_thread_state;

typedef struct {
  uint32_t crtc_id;
  uint32_t crtc_pipe;

  int width;
  int height;

  drm_plane *plane;
  uint32_t prefer_plane_id;

  drm_cursor_state cursor_next;
  drm_cursor_state cursor_curr;

  pthread_t thread;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  drm_thread_state state;

  void *egl_ctx;

  int verified;

  int use_afbc_modifier;
  int blocked;
  int async_commit;

  uint64_t last_update_time;
} drm_crtc;

typedef struct {
  int fd;

  drm_crtc crtcs[DRM_MAX_CRTCS];
  int num_crtcs;

  drmModePlaneResPtr pres;
  drmModeRes *res;

  int prefer_afbc_modifier;
  int allow_overlay;
  int num_surfaces;
  int inited;
  int atomic;
  int hide;
  uint64_t min_interval;

  float scale_x, scale_y;
  float scale_from;

  char *configs;
} drm_ctx;

static drm_ctx g_drm_ctx = { 0, };
drm_private int g_drm_debug = 0;
drm_private FILE *g_log_fp = NULL;

static inline uint64_t drm_curr_time(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec / 1000;
}

static int drm_plane_get_prop(drm_ctx *ctx, drm_plane *plane, drm_plane_prop p)
{
  drmModePropertyPtr prop;
  uint32_t i;

  if (plane->prop_ids[p])
    return plane->prop_ids[p];

  for (i = 0; i < plane->props->count_props; i++) {
    prop = drmModeGetProperty(ctx->fd, plane->props->props[i]);
    if (prop && !strcmp(prop->name, drm_plane_prop_names[p])) {
      drmModeFreeProperty(prop);
      plane->prop_ids[p] = i;
      return i;
    }
    drmModeFreeProperty(prop);
  }

  return -1;
}

static int drm_atomic_add_plane_prop(drm_ctx *ctx, drmModeAtomicReq *request,
                                     drm_plane *plane, drm_plane_prop p,
                                     uint64_t value)
{
  int prop_idx = drm_plane_get_prop(ctx, plane, p);
  if (prop_idx < 0)
    return -1;

  return drmModeAtomicAddProperty(request, plane->plane_id,
                                  plane->props->props[prop_idx], value);
}

static int drm_set_plane(drm_ctx *ctx, drm_crtc *crtc, drm_plane *plane,
                         uint32_t fb, int x, int y, int w, int h)
{
  drmModeAtomicReq *req;
  int ret = 0;

  if (plane->cursor_plane || crtc->async_commit || !ctx->atomic)
    goto legacy;

  req = drmModeAtomicAlloc();
  if (!req)
    goto legacy;

  if (!fb) {
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_CRTC_ID, 0);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_FB_ID, 0);
  } else {
    ret |= drm_atomic_add_plane_prop(ctx, req, plane,
                                     PLANE_PROP_CRTC_ID, crtc->crtc_id);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_FB_ID, fb);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_SRC_X, 0);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_SRC_Y, 0);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane,
                                     PLANE_PROP_SRC_W, w << 16);
    ret |= drm_atomic_add_plane_prop(ctx, req,
                                     plane, PLANE_PROP_SRC_H, h << 16);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_CRTC_X, x);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_CRTC_Y, y);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_CRTC_W, w);
    ret |= drm_atomic_add_plane_prop(ctx, req, plane, PLANE_PROP_CRTC_H, h);
  }

  ret |= drmModeAtomicCommit(ctx->fd, req, DRM_MODE_ATOMIC_NONBLOCK, NULL);
  drmModeAtomicFree(req);

  if (ret >= 0)
    return 0;

legacy:
  if (ret < 0 && ctx->atomic) {
    DRM_ERROR("CRTC[%d]: failed to do atomic commit (%d)\n",
              crtc->crtc_id, errno);
    ctx->atomic = 0;
  }
  return drmModeSetPlane(ctx->fd, plane->plane_id, crtc->crtc_id, fb, 0,
                         x, y, w, h, 0, 0, w << 16, h << 16);
}

static int drm_plane_get_prop_value(drm_ctx *ctx, drm_plane *plane,
                                    drm_plane_prop p, uint64_t *value)
{
  int prop_idx = drm_plane_get_prop(ctx, plane, p);
  if (prop_idx < 0)
    return -1;

  *value = plane->props->prop_values[prop_idx];
  return 0;
}

static int drm_plane_set_prop_max(drm_ctx *ctx, drm_plane *plane,
                                  drm_plane_prop p)
{
  drmModePropertyPtr prop;
  int prop_idx = drm_plane_get_prop(ctx, plane, p);
  if (prop_idx < 0)
    return -1;

  prop = drmModeGetProperty(ctx->fd, plane->props->props[prop_idx]);
  drmModeObjectSetProperty (ctx->fd, plane->plane_id,
                            DRM_MODE_OBJECT_PLANE,
                            plane->props->props[prop_idx],
                            prop->values[prop->count_values - 1]);
  DRM_DEBUG("set plane %d prop: %s to max: %"PRIu64"\n",
            plane->plane_id, drm_plane_prop_names[p],
            prop->values[prop->count_values - 1]);
  drmModeFreeProperty(prop);
  return 0;
}

static void drm_free_plane(drm_plane *plane)
{
  drmModeFreeObjectProperties(plane->props);
  drmModeFreePlane(plane->plane);
  free(plane);
}

static void drm_plane_update_format(drm_ctx *ctx, drm_plane *plane)
{
  drmModePropertyBlobPtr blob;
  struct drm_format_modifier_blob *header;
  struct drm_format_modifier *modifiers;
  uint32_t *formats;
  uint64_t value;
  uint32_t i, j;

  plane->can_afbc = plane->can_linear = 0;

  /* Check formats */
  for (i = 0; i < plane->plane->count_formats; i++) {
    if (plane->plane->formats[i] == DRM_FORMAT_ARGB8888)
      break;
  }
  if (i == plane->plane->count_formats)
    return;

  if (drm_plane_get_prop_value(ctx, plane, PLANE_PROP_IN_FORMATS, &value) < 0) {
    /* No in_formats */
    plane->can_linear = 1;
    return;
  }

  blob = drmModeGetPropertyBlob(ctx->fd, value);
  if (!blob)
    return;

  header = blob->data;
  formats = (uint32_t *) ((char *) header + header->formats_offset);
  modifiers = (struct drm_format_modifier *)
    ((char *) header + header->modifiers_offset);

  /* Check in_formats */
  for (i = 0; i < header->count_formats; i++) {
    if (formats[i] == DRM_FORMAT_ARGB8888)
      break;
  }
  if (i == header->count_formats)
    goto out;

  if (!header->count_modifiers) {
    plane->can_linear = 1;
    goto out;
  }

  /* Check modifiers */
  for (j = 0; j < header->count_modifiers; j++) {
    struct drm_format_modifier *mod = &modifiers[j];

    if ((i < mod->offset) || (i > mod->offset + 63))
      continue;
    if (!(mod->formats & (1 << (i - mod->offset))))
      continue;

    if (mod->modifier == DRM_AFBC_MODIFIER)
      plane->can_afbc = 1;

    if (mod->modifier == DRM_FORMAT_MOD_LINEAR)
      plane->can_linear = 1;
  }

out:
  drmModeFreePropertyBlob(blob);
}

static drm_plane *drm_get_plane(drm_ctx *ctx, uint32_t plane_id)
{
  drm_plane *plane = calloc(1, sizeof(*plane));
  if (!plane)
    return NULL;

  plane->plane_id = plane_id;
  plane->plane = drmModeGetPlane(ctx->fd, plane_id);
  if (!plane->plane)
    goto err;

  plane->props = drmModeObjectGetProperties(ctx->fd, plane_id,
                                            DRM_MODE_OBJECT_PLANE);
  if (!plane->props)
    goto err;

  drm_plane_update_format(ctx, plane);
  return plane;
err:
  drm_free_plane(plane);
  return NULL;
}

static void drm_load_configs(drm_ctx *ctx)
{
  struct stat st;
  const char *file = DRM_CURSOR_CONFIG_FILE;
  char *ptr, *tmp;
  int fd;

  if (stat(file, &st) < 0)
    return;

  fd = open(file, O_RDONLY);
  if (fd < 0)
    return;

  ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (ptr == MAP_FAILED)
    goto out_close_fd;

  ctx->configs = malloc(st.st_size + 1);
  if (!ctx->configs)
    goto out_unmap;

  memcpy(ctx->configs, ptr, st.st_size);
  ctx->configs[st.st_size] = '\0';

  tmp = ctx->configs;
  while ((tmp = strchr(tmp, '#'))) {
    while (*tmp != '\n' && *tmp != '\0')
      *tmp++ = '\n';
  }

out_unmap:
  munmap(ptr, st.st_size);
out_close_fd:
  close(fd);
}

static const char *drm_get_config(drm_ctx *ctx, const char *name)
{
  static char buf[4096];
  const char *config;

  if (!ctx->configs)
    return NULL;

  config = strstr(ctx->configs, name);
  if (!config)
    return NULL;

  if (config[strlen(name)] == '\n' || config[strlen(name)] == '\r')
    return NULL;

  sscanf(config + strlen(name), "%4095s", buf);
  return buf;
}

static int drm_get_config_int(drm_ctx *ctx, const char *name, int def)
{
  const char *config = drm_get_config(ctx, name);

  if (config)
    return atoi(config);

  return def;
}

static drm_ctx *drm_get_ctx(int fd)
{
  drm_ctx *ctx = &g_drm_ctx;
  uint32_t prefer_planes[DRM_MAX_CRTCS] = { 0, };
  uint32_t prefer_plane = 0;
  uint32_t i, max_fps, count_crtcs;
  const char *config;

  if (fd < 0)
    return ctx;

  if (ctx->inited) {
    /* Make sure the ctx's fd is the same as the input fd */
    int flags = fcntl(ctx->fd, F_GETFL, 0);
    if (fcntl(fd, F_GETFL, 0) == flags) {
      fcntl(ctx->fd, F_SETFL, flags ^ O_NONBLOCK);
      if (fcntl(fd, F_GETFL, 0) != flags) {
        fcntl(ctx->fd, F_SETFL, flags);
        return ctx;
      }
    }

    close(ctx->fd);
    ctx->fd = dup(fd);
    return ctx;
  }

  /* Failed already */
  if (ctx->fd < 0)
    return NULL;

  ctx->fd = dup(fd);
  if (ctx->fd < 0)
    return NULL;

  drm_load_configs(ctx);

  g_drm_debug = drm_get_config_int(ctx, OPT_DEBUG, 0);

  if (getenv("DRM_DEBUG") || !access("/tmp/.drm_cursor_debug", F_OK))
    g_drm_debug = 1;

  if (!(config = getenv("DRM_CURSOR_LOG_FILE")))
    config = drm_get_config(ctx, OPT_LOG_FILE);

  g_log_fp = fopen(config ? config : "/var/log/drm-cursor.log", "wb+");

  ctx->atomic = drm_get_config_int(ctx, OPT_ATOMIC, 1);
  DRM_INFO("atomic drm API %s\n", ctx->atomic ? "enabled" : "disabled");

  ctx->hide = drm_get_config_int(ctx, OPT_HIDE, 0);
  if (ctx->hide)
    DRM_INFO("invisible cursors\n");

#ifdef PREFER_AFBC_MODIFIER
  ctx->prefer_afbc_modifier = 1;
#endif

  ctx->prefer_afbc_modifier =
    drm_get_config_int(ctx, OPT_PREFER_AFBC, ctx->prefer_afbc_modifier);

  if (ctx->prefer_afbc_modifier)
    DRM_DEBUG("prefer ARM AFBC modifier\n");

  ctx->allow_overlay = drm_get_config_int(ctx, OPT_ALLOW_OVERLAY, 0);

  if (ctx->allow_overlay)
    DRM_DEBUG("allow overlay planes\n");

  drmSetClientCap(ctx->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

  ctx->num_surfaces = drm_get_config_int(ctx, OPT_NUM_SURFACES, 8);

  max_fps = drm_get_config_int(ctx, OPT_MAX_FPS, 0);
  if (max_fps <= 0)
    max_fps = 60;

  ctx->min_interval = 1000 / max_fps;
  if (ctx->min_interval)
    ctx->min_interval--;

  DRM_INFO("max fps: %d\n", max_fps);

  config = drm_get_config(ctx, OPT_SCALE_FROM);
  if (config) {
    int w, h, screen_w, screen_h;
    if (config &&
        sscanf(config, "%dx%d/%dx%d", &w, &h, &screen_w, &screen_h) == 4) {
      ctx->scale_from = 1.0 * w * h / screen_w / screen_h;
      DRM_INFO("scale from: %s\n", config);
    }
  } else {
    config = drm_get_config(ctx, OPT_SCALE);
    if (config && sscanf(config, "%fx%f", &ctx->scale_x, &ctx->scale_y) == 2)
      DRM_INFO("scale: %s\n", config);
  }

  ctx->res = drmModeGetResources(ctx->fd);
  if (!ctx->res)
    goto err_free_configs;

  ctx->pres = drmModeGetPlaneResources(ctx->fd);
  if (!ctx->pres)
    goto err_free_res;

  count_crtcs = ctx->res->count_crtcs;

  /* Allow specifying prefer plane */
  if ((config = getenv("DRM_CURSOR_PREFER_PLANE")))
    prefer_plane = atoi(config);
  else
    prefer_plane = drm_get_config_int(ctx, OPT_PREFER_PLANE, 0);

  /* Allow specifying prefer planes */
  if (!(config = getenv("DRM_CURSOR_PREFER_PLANES")))
    config = drm_get_config(ctx, OPT_PREFER_PLANES);
  for (i = 0; config && i < count_crtcs; i++) {
    prefer_planes[i] = atoi(config);

    config = strchr(config, ',');
    if (config)
      config++;
  }

  /* Fetch all CRTCs */
  for (i = 0; i < count_crtcs; i++) {
    drmModeCrtcPtr c = drmModeGetCrtc(ctx->fd, ctx->res->crtcs[i]);
    drm_crtc *crtc = &ctx->crtcs[ctx->num_crtcs];

    if (!c)
      continue;

    crtc->crtc_id = c->crtc_id;
    crtc->crtc_pipe = i;
    crtc->prefer_plane_id = prefer_planes[i] ? prefer_planes[i] : prefer_plane;

    DRM_DEBUG("found %d CRTC: %d(%d) (%dx%d) prefer plane: %d\n",
              ctx->num_crtcs, c->crtc_id, i, c->width, c->height,
              crtc->prefer_plane_id);

    ctx->num_crtcs++;
    drmModeFreeCrtc(c);
  }

  DRM_DEBUG("found %d CRTCs\n", ctx->num_crtcs);

  if (!ctx->num_crtcs)
    goto err_free_pres;

  config = drm_get_config(ctx, OPT_CRTC_BLOCKLIST);
  for (i = 0; config && i < count_crtcs; i++) {
    uint32_t crtc_id = atoi(config);

    for (int j = 0; j < ctx->num_crtcs; j++) {
      drm_crtc *crtc = &ctx->crtcs[j];
      if (crtc->crtc_id != crtc_id)
        continue;

      DRM_DEBUG("CRTC: %d blocked\n", crtc_id);
      crtc->blocked = 1;
    }

    config = strchr(config, ',');
    if (config)
      config++;
  }

  if (g_drm_debug) {
    /* Dump planes for debugging */
    for (i = 0; i < ctx->pres->count_planes; i++) {
      drm_plane *plane = drm_get_plane(ctx, ctx->pres->planes[i]);
      char *type;
      uint64_t value = 0;

      if (!plane)
        continue;

      drm_plane_get_prop_value(ctx, plane, PLANE_PROP_type, &value);
      switch (value) {
      case DRM_PLANE_TYPE_PRIMARY:
        type = "primary";
        break;
      case DRM_PLANE_TYPE_OVERLAY:
        type = "overlay";
        break;
      case DRM_PLANE_TYPE_CURSOR:
        type = "cursor ";
        break;
      default:
        type = "unknown";
        break;
      }

      DRM_DEBUG("found plane: %d[%s] crtcs: 0x%x %s%s\n",
                plane->plane_id, type, plane->plane->possible_crtcs,
                plane->can_linear ? "(ARGB)" : "",
                plane->can_afbc ? "(AFBC)" : "");

      drm_free_plane(plane);
    }
  }

  DRM_INFO("using libdrm-cursor (%s)\n", LIBDRM_CURSOR_VERSION);

  ctx->inited = 1;
  return ctx;

err_free_pres:
  drmModeFreePlaneResources(ctx->pres);
err_free_res:
  drmModeFreeResources(ctx->res);
err_free_configs:
  free(ctx->configs);
  close(ctx->fd);
  ctx->fd = -1;
  return NULL;
}

#define drm_crtc_bind_plane_force(ctx, crtc, plane) \
  drm_crtc_bind_plane(ctx, crtc, plane, 1)

#define drm_crtc_bind_plane_cursor(ctx, crtc, plane) \
  drm_crtc_bind_plane(ctx, crtc, plane, 0)

static int drm_crtc_bind_plane(drm_ctx *ctx, drm_crtc *crtc, uint32_t plane_id,
                               int allow_overlay)
{
  drm_plane *plane;
  uint64_t value;
  int i;

  /* CRTC already assigned */
  if (crtc->plane)
    return 1;

  /* Plane already assigned */
  for (i = 0; i < ctx->num_crtcs; i++) {
    if (ctx->crtcs[i].plane && ctx->crtcs[i].plane->plane_id == plane_id)
      return -1;
  }

  plane = drm_get_plane(ctx, plane_id);
  if (!plane)
    return -1;

  /* Unable to use */
  if (!plane->can_afbc && !plane->can_linear)
    goto err;

  /* Not for this CRTC */
  if (!(plane->plane->possible_crtcs & (1 << crtc->crtc_pipe)))
    goto err;

  /* Not using primary planes */
  if (drm_plane_get_prop_value(ctx, plane, PLANE_PROP_type, &value) < 0)
    goto err;

  if (value == DRM_PLANE_TYPE_PRIMARY)
    goto err;

  /* Check for overlay plane */
  if (!allow_overlay && value == DRM_PLANE_TYPE_OVERLAY)
    goto err;

  plane->cursor_plane = value == DRM_PLANE_TYPE_CURSOR;
  if (plane->cursor_plane)
    DRM_INFO("CRTC[%d]: using cursor plane\n", crtc->crtc_id);

  if (ctx->prefer_afbc_modifier && plane->can_afbc)
    crtc->use_afbc_modifier = 1;
  else if (!plane->can_linear)
    crtc->use_afbc_modifier = 1;

  DRM_DEBUG("CRTC[%d]: bind plane: %d%s\n", crtc->crtc_id, plane->plane_id,
            crtc->use_afbc_modifier ? "(AFBC)" : "");

  crtc->plane = plane;

  return 0;
err:
  drm_free_plane(plane);
  return -1;
}

static int drm_crtc_valid(drm_crtc *crtc)
{
  return (crtc->width > 0 && crtc->height > 0) ? 0 : -1;
}

static int drm_update_crtc(drm_ctx *ctx, drm_crtc *crtc)
{
  drmModeCrtcPtr c;
  int was_connected, connected;

  c = drmModeGetCrtc(ctx->fd, crtc->crtc_id);
  if (!c)
    return -1;

  was_connected = drm_crtc_valid(crtc) >= 0;
  crtc->width = c->width;
  crtc->height = c->height;
  connected = drm_crtc_valid(crtc) >= 0;

  drmModeFreeCrtc(c);

  if (connected != was_connected)
    DRM_DEBUG("CRTC[%d]: %s!\n", crtc->crtc_id, \
              connected ? "connected" : "disconnected");

  return drm_crtc_valid(crtc);
}

static int drm_crtc_update_offsets(drm_ctx *ctx, drm_crtc *crtc,
                                   drm_cursor_state *cursor_state)
{
  int x, y, off_x, off_y, width, height, area_w, area_h;
  float scale_x, scale_y;

  if (drm_update_crtc(ctx, crtc) < 0)
    return -1;

  width = cursor_state->width;
  height = cursor_state->height;

  if (ctx->scale_from) {
    scale_x = scale_y =
      ctx->scale_from * crtc->width * crtc->height / width / height;
  } else {
    scale_x = ctx->scale_x ? ctx->scale_x : 1.0;
    scale_y = ctx->scale_y ? ctx->scale_y : 1.0;
  }

  width *= scale_x;
  height *= scale_y;

  x = cursor_state->x + cursor_state->hot_x - cursor_state->hot_x * scale_x;
  y = cursor_state->y + cursor_state->hot_y - cursor_state->hot_y * scale_y;
  area_w = crtc->width - width;
  area_h = crtc->height - height;

  off_x = off_y = 0;

  if (x < 0)
    off_x = x;

  if (y < 0)
    off_y = y;

  if (x > area_w)
    off_x = x - area_w;

  if (y > area_h)
    off_y = y - area_h;

  cursor_state->scaled_x = x;
  cursor_state->scaled_y = y;
  cursor_state->off_x = off_x;
  cursor_state->off_y = off_y;
  cursor_state->scaled_w = width;
  cursor_state->scaled_h = height;

  return 0;
}

#define drm_crtc_disable_cursor(ctx, crtc) \
  drm_crtc_update_cursor(ctx, crtc, NULL)

static int drm_crtc_update_cursor(drm_ctx *ctx, drm_crtc *crtc,
                                  drm_cursor_state *cursor_state)
{
  drm_plane *plane = crtc->plane;
  uint32_t old_fb = crtc->cursor_curr.fb;
  uint32_t fb;
  int x, y, w, h, ret;

  /* Disable */
  if (!cursor_state) {
    if (old_fb) {
      DRM_DEBUG("CRTC[%d]: disabling cursor\n", crtc->crtc_id);
      drm_set_plane(ctx, crtc, plane, 0, 0, 0, 0, 0);
      drmModeRmFB(ctx->fd, old_fb);
    }

    memset(&crtc->cursor_curr, 0, sizeof(drm_cursor_state));
    return 0;
  }

  /* Unchanged */
  if (crtc->cursor_curr.fb == cursor_state->fb &&
      crtc->cursor_curr.scaled_x == cursor_state->scaled_x &&
      crtc->cursor_curr.scaled_y == cursor_state->scaled_y &&
      crtc->cursor_curr.off_x == cursor_state->off_x &&
      crtc->cursor_curr.off_y == cursor_state->off_y) {
    crtc->cursor_curr = *cursor_state;
    return 0;
  }

  fb = cursor_state->fb;
  x = cursor_state->scaled_x - cursor_state->off_x;
  y = cursor_state->scaled_y - cursor_state->off_y;
  w = cursor_state->scaled_w;
  h = cursor_state->scaled_h;

  DRM_DEBUG("CRTC[%d]: setting fb: %d (%dx%d) on plane: %d at (%d,%d)\n",
            crtc->crtc_id, fb, w, h, plane->plane_id, x, y);

  ret = drm_set_plane(ctx, crtc, plane, fb, x, y, w, h);
  if (ret)
    DRM_ERROR("CRTC[%d]: failed to set plane (%d)\n", crtc->crtc_id, errno);

  if (old_fb && old_fb != fb) {
    DRM_DEBUG("CRTC[%d]: remove FB: %d\n", crtc->crtc_id, old_fb);
    drmModeRmFB(ctx->fd, old_fb);
  }

  crtc->cursor_curr = *cursor_state;
  return ret;
}

static int drm_crtc_create_fb(drm_ctx *ctx, drm_crtc *crtc,
                              drm_cursor_state *cursor_state)
{
  uint32_t handle = cursor_state->handle;
  int width = cursor_state->width;
  int height = cursor_state->height;
  int scaled_w = cursor_state->scaled_w;
  int scaled_h = cursor_state->scaled_h;
  int off_x = cursor_state->off_x;
  int off_y = cursor_state->off_y;

  DRM_DEBUG("CRTC[%d]: convert FB from %d (%dx%d) to (%dx%d) offset: (%d,%d)\n",
            crtc->crtc_id, handle, width, height,
            scaled_w, scaled_h, off_x, off_y);

  if (!crtc->egl_ctx) {
    uint64_t modifier;
    int format;

    if (crtc->use_afbc_modifier) {
      /* Mali only support AFBC with BGR formats now */
      format = GBM_FORMAT_ABGR8888;
      modifier = DRM_AFBC_MODIFIER;
    } else {
      format = GBM_FORMAT_ARGB8888;
      modifier = 0;
    }

    crtc->egl_ctx = egl_init_ctx(ctx->fd, ctx->num_surfaces, format, modifier);
    if (!crtc->egl_ctx) {
      DRM_ERROR("CRTC[%d]: failed to init egl ctx\n", crtc->crtc_id);
      return -1;
    }
  }

  cursor_state->fb =
    egl_convert_fb(ctx->fd, crtc->egl_ctx, handle, width, height,
                   scaled_w, scaled_h, off_x, off_y);
  if (!cursor_state->fb) {
    DRM_ERROR("CRTC[%d]: failed to create FB\n", crtc->crtc_id);
    return -1;
  }

  DRM_DEBUG("CRTC[%d]: created FB: %d\n", crtc->crtc_id, cursor_state->fb);
  return 0;
}

static void *drm_crtc_thread_fn(void *data)
{
  drm_ctx *ctx = drm_get_ctx(-1);
  drm_crtc *crtc = data;
  drm_plane *plane = crtc->plane;
  drm_cursor_state cursor_state;
  uint64_t duration;
  char name[256];

  DRM_DEBUG("CRTC[%d]: thread started\n", crtc->crtc_id);

  /**
   * The new DRM driver doesn't allow setting atomic cap for Xorg.
   * Let's use a custom thread name to workaround that.
   */
  snprintf(name, sizeof(name), "drm-cursor[%d]", crtc->crtc_id);
  pthread_setname_np(crtc->thread, name);

  if (!plane->cursor_plane) {
    drmSetClientCap(ctx->fd, DRM_CLIENT_CAP_ATOMIC, 1);

    /* Reflush props with atomic cap enabled */
    drmModeFreeObjectProperties(plane->props);
    plane->props = drmModeObjectGetProperties(ctx->fd, plane->plane_id,
                                              DRM_MODE_OBJECT_PLANE);
    if (!plane->props)
      goto error;

    /* Set maximum ZPOS */
    drm_plane_set_prop_max(ctx, plane, PLANE_PROP_zpos);
    drm_plane_set_prop_max(ctx, plane, PLANE_PROP_ZPOS);

    /* Set async commit for Rockchip BSP kernel */
    crtc->async_commit =
      !drm_plane_set_prop_max(ctx, plane, PLANE_PROP_ASYNC_COMMIT);
    if (crtc->async_commit)
      DRM_INFO("CRTC[%d]: using async commit\n", crtc->crtc_id);
  }

  crtc->last_update_time = drm_curr_time();

  while (1) {
    /* Wait for new cursor state */
    pthread_mutex_lock(&crtc->mutex);
    while (crtc->state != PENDING)
      pthread_cond_wait(&crtc->cond, &crtc->mutex);

    cursor_state = crtc->cursor_next;
    crtc->cursor_next.request = 0;
    crtc->state = IDLE;
    cursor_state.request |= crtc->cursor_curr.request; /* For retry */
    pthread_mutex_unlock(&crtc->mutex);

    /* For edge moving */
    if (drm_crtc_update_offsets(ctx, crtc, &cursor_state) < 0) {
      DRM_DEBUG("CRTC[%d]: unavailable!\n", crtc->crtc_id);
      drm_crtc_disable_cursor(ctx, crtc);
      goto retry;
    }

    if (cursor_state.request & REQ_SET_CURSOR) {
      cursor_state.request = 0;

      /* Handle set-cursor */
      DRM_DEBUG("CRTC[%d]: set new cursor %d (%dx%d)\n",
                crtc->crtc_id, cursor_state.handle,
                cursor_state.width, cursor_state.height);

      if (!cursor_state.handle) {
        drm_crtc_disable_cursor(ctx, crtc);
        goto next;
      }

      if (drm_crtc_create_fb(ctx, crtc, &cursor_state) < 0)
        goto error;

      if (drm_crtc_update_cursor(ctx, crtc, &cursor_state) < 0) {
        DRM_ERROR("CRTC[%d]: failed to set cursor\n", crtc->crtc_id);
        goto error;
      }
    } else if (cursor_state.request & REQ_MOVE_CURSOR) {
      cursor_state.request = 0;

      /* Handle move-cursor */
      DRM_DEBUG("CRTC[%d]: move cursor to (%d[%d],%d[%d])\n",
                crtc->crtc_id, cursor_state.scaled_x, -cursor_state.off_x,
                cursor_state.scaled_y, -cursor_state.off_y);

      if (!crtc->cursor_curr.handle) {
        /* Pre-moving */
        crtc->cursor_curr = cursor_state;
        goto next;
      } else if (crtc->cursor_curr.off_x != cursor_state.off_x ||
                 crtc->cursor_curr.off_y != cursor_state.off_y) {
        /* Edge moving */
        if (drm_crtc_create_fb(ctx, crtc, &cursor_state) < 0)
          goto error;
      } else {
        /* Normal moving */
        cursor_state.fb = crtc->cursor_curr.fb;
      }

      if (drm_crtc_update_cursor(ctx, crtc, &cursor_state) < 0) {
        DRM_ERROR("CRTC[%d]: failed to move cursor\n", crtc->crtc_id);
        goto error;
      }
    }

    if (!crtc->verified && crtc->cursor_curr.fb) {
      pthread_mutex_lock(&crtc->mutex);
      DRM_INFO("CRTC[%d]: it works!\n", crtc->crtc_id);
      crtc->verified = 1;
      pthread_cond_signal(&crtc->cond);
      pthread_mutex_unlock(&crtc->mutex);
    }

next:
    duration = drm_curr_time() - crtc->last_update_time;
    if (duration < ctx->min_interval)
      usleep((ctx->min_interval - duration) * 1000);
    crtc->last_update_time = drm_curr_time();;
    continue;
retry:
    /* Force setting cursor in next request */
    pthread_mutex_lock(&crtc->mutex);
    crtc->cursor_curr.request = REQ_SET_CURSOR;
    pthread_cond_signal(&crtc->cond);
    pthread_mutex_unlock(&crtc->mutex);
    goto next;
  }

error:
  if (crtc->egl_ctx)
    egl_free_ctx(crtc->egl_ctx);

  drm_crtc_disable_cursor(ctx, crtc);

  pthread_mutex_lock(&crtc->mutex);
  DRM_DEBUG("CRTC[%d]: thread error\n", crtc->crtc_id);
  crtc->state = FATAL_ERROR;

  if (crtc->plane) {
    drm_free_plane(crtc->plane);
    crtc->plane = NULL;
  }

  pthread_cond_signal(&crtc->cond);
  pthread_mutex_unlock(&crtc->mutex);

  return NULL;
}

static int drm_crtc_prepare(drm_ctx *ctx, drm_crtc *crtc)
{
  uint32_t i;

  /* Update CRTC if unavailable */
  if (drm_crtc_valid(crtc) < 0)
    drm_update_crtc(ctx, crtc);

  /* CRTC already assigned */
  if (crtc->plane)
    return 1;

  /* Try specific plane */
  if (crtc->prefer_plane_id)
    drm_crtc_bind_plane_force(ctx, crtc, crtc->prefer_plane_id);

  /* Try cursor plane */
  for (i = 0; !crtc->plane && i < ctx->pres->count_planes; i++)
    drm_crtc_bind_plane_cursor(ctx, crtc, ctx->pres->planes[i]);

  /* Fallback to any available overlay plane */
  if (ctx->allow_overlay) {
    for (i = ctx->pres->count_planes; !crtc->plane && i; i--)
      drm_crtc_bind_plane_force(ctx, crtc, ctx->pres->planes[i - 1]);
  }

  if (!crtc->plane) {
    DRM_ERROR("CRTC[%d]: failed to find any plane\n", crtc->crtc_id);
    return -1;
  }

  crtc->state = IDLE;

  pthread_cond_init(&crtc->cond, NULL);
  pthread_mutex_init(&crtc->mutex, NULL);
  pthread_create(&crtc->thread, NULL, drm_crtc_thread_fn, crtc);

  return 0;
}

static drm_crtc *drm_get_crtc(drm_ctx *ctx, uint32_t crtc_id)
{
  drm_crtc *crtc = NULL;
  int i;

  for (i = 0; i < ctx->num_crtcs; i++) {
    crtc = &ctx->crtcs[i];
    if (!crtc_id && drm_update_crtc(ctx, crtc) < 0)
      continue;

    if (crtc->blocked)
      continue;

    if (!crtc_id || crtc->crtc_id == crtc_id)
      break;
  }

  if (i == ctx->num_crtcs) {
    DRM_ERROR("CRTC[%d]: not available\n", crtc_id);
    return NULL;
  }

  return crtc;
}

static int drm_set_cursor(int fd, uint32_t crtc_id, uint32_t handle,
                          uint32_t width, uint32_t height,
                          int hot_x, int hot_y)
{
  drm_crtc *crtc;
  drm_ctx *ctx;
  drm_cursor_state *cursor_next;

  ctx = drm_get_ctx(fd);
  if (!ctx)
    return -1;

  if (ctx->hide)
    return 0;

  crtc = drm_get_crtc(ctx, crtc_id);
  if (!crtc)
    return -1;

  if (drm_crtc_prepare(ctx, crtc) < 0)
    return -1;

  DRM_DEBUG("CRTC[%d]: request setting new cursor %d (%dx%d)\n",
            crtc->crtc_id, handle, width, height);

  pthread_mutex_lock(&crtc->mutex);
  if (crtc->state == FATAL_ERROR) {
    pthread_mutex_unlock(&crtc->mutex);
    DRM_ERROR("CRTC[%d]: failed to set cursor\n", crtc->crtc_id);
    return -1;
  }

  /* Update next cursor state and notify the thread */
  cursor_next = &crtc->cursor_next;

  crtc->cursor_curr.request = 0;
  cursor_next->request = REQ_SET_CURSOR;

  cursor_next->fb = 0;
  cursor_next->handle = handle;
  cursor_next->width = width;
  cursor_next->height = height;
  cursor_next->hot_x = hot_x;
  cursor_next->hot_y = hot_y;
  crtc->state = PENDING;
  pthread_cond_signal(&crtc->cond);

  if (handle) {
    /**
     * Wait for verified or fatal error or retry.
     * HACK: Fake retry as successed.
     */
    while (!crtc->verified && crtc->state != FATAL_ERROR && \
           !crtc->cursor_curr.request)
      pthread_cond_wait(&crtc->cond, &crtc->mutex);
  }

  pthread_mutex_unlock(&crtc->mutex);

  if (crtc->state == FATAL_ERROR) {
    DRM_ERROR("CRTC[%d]: failed to set cursor\n", crtc->crtc_id);
    return -1;
  }

  return 0;
}

static int drm_move_cursor(int fd, uint32_t crtc_id, int x, int y)
{
  drm_ctx *ctx;
  drm_crtc *crtc;
  drm_cursor_state *cursor_next;

  ctx = drm_get_ctx(fd);
  if (!ctx)
    return -1;

  if (ctx->hide)
    return 0;

  crtc = drm_get_crtc(ctx, crtc_id);
  if (!crtc)
    return -1;

  if (crtc->state == FATAL_ERROR || drm_crtc_prepare(ctx, crtc) < 0)
    return -1;

  if (drm_crtc_valid(crtc) < 0)
    return -1;

  DRM_DEBUG("CRTC[%d]: request moving cursor to (%d,%d) in (%dx%d)\n",
            crtc->crtc_id, x, y, crtc->width, crtc->height);

  pthread_mutex_lock(&crtc->mutex);
  if (crtc->state == FATAL_ERROR) {
    pthread_mutex_unlock(&crtc->mutex);
    return -1;
  }

  /* Update next cursor state and notify the thread */
  cursor_next = &crtc->cursor_next;

  cursor_next->request |= REQ_MOVE_CURSOR;
  cursor_next->fb = 0;
  cursor_next->x = x;
  cursor_next->y = y;
  crtc->state = PENDING;
  pthread_cond_signal(&crtc->cond);
  pthread_mutex_unlock(&crtc->mutex);

  return 0;
}

/* Hook functions */

int drmModeSetCursor2(int fd, uint32_t crtcId, uint32_t bo_handle,
                      uint32_t width, uint32_t height,
                      int32_t hot_x, int32_t hot_y)
{
  /* Init log file */
  drm_get_ctx(fd);

  DRM_DEBUG("fd: %d crtc: %d handle: %d size: %dx%d (%d, %d)\n",
            fd, crtcId, bo_handle, width, height, hot_x, hot_y);
  return drm_set_cursor(fd, crtcId, bo_handle, width, height, hot_x, hot_y);
}

int drmModeSetCursor(int fd, uint32_t crtcId, uint32_t bo_handle,
                     uint32_t width, uint32_t height)
{
  drm_ctx *ctx;

  ctx = drm_get_ctx(fd);
  if (!ctx)
    return -1;

  DRM_DEBUG("fd: %d crtc: %d handle: %d size: %dx%d\n",
            fd, crtcId, bo_handle, width, height);

  if (bo_handle && width && height &&
      (ctx->scale_from || ctx->scale_x || ctx->scale_y))
    DRM_INFO("CRTC[%d]: scaling without hotspots, use drmModeSetCursor2()!\n",
             crtcId);

  return drm_set_cursor(fd, crtcId, bo_handle, width, height, 0, 0);
}

int drmModeMoveCursor(int fd, uint32_t crtcId, int x, int y)
{
  DRM_DEBUG("fd: %d crtc: %d position: %d,%d\n", fd, crtcId, x, y);
  return drm_move_cursor(fd, crtcId, x, y);
}
