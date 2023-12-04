/*
 * Copyright (C) 2019 Hertz Wang 1989wanghang@163.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see http://www.gnu.org/licenses
 *
 * Any non-GPL usage of this software or parts of this software is strictly
 * forbidden.
 *
 */

#define IN_RKNN_SSD_CC

#include <assert.h>

#include "npu_pp_output.h"

namespace NPU_UVC_SSD_DEMO {

#include "../../../../../../../external/rknpu/rknn/rknn_api/examples/rknn_ssd_demo/src/ssd.cc"

static SDLFont sdl_font(red, 16);
bool SSDDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
             const SDL_Rect &coor_rect, int rotate,
             NPUPostProcessOutput *output, void *buffer, Uint32 sdl_fmt);
bool SSDDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
             const SDL_Rect &coor_rect, int rotate,
             NPUPostProcessOutput *output, void *buffer, Uint32 sdl_fmt) {
  int npu_w = output->npuwh.width;
  int npu_h = output->npuwh.height;
  auto group = (NPU_UVC_SSD_DEMO::detect_result_group_t *)(output->pp_output);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0xFF, 0x10, 0xEB, 0xFF);
  for (int i = 0; i < group->count; i++) {
    detect_result_t *det_result = &(group->results[i]);
    printf("%s @ (%d %d %d %d) %f\n", det_result->name, det_result->box.left,
           det_result->box.top, det_result->box.right, det_result->box.bottom,
           det_result->prop);
    int x1 = det_result->box.left;
    int y1 = det_result->box.top;
    int x2 = det_result->box.right;
    int y2 = det_result->box.bottom;
    SDL_Rect rect = {x1 * render_rect.w / npu_w + render_rect.x,
                     y1 * render_rect.h / npu_h + render_rect.y,
                     (x2 - x1) * render_rect.w / npu_w,
                     (y2 - y1) * render_rect.h / npu_h};
    SDL_Rect line_rect = transform(rect, coor_rect, rotate);
    int status = draw_rect(renderer, &line_rect, buffer, sdl_fmt);
    if (status)
      fprintf(stderr, "draw rect status: %d <%s>\n", status, SDL_GetError());
    if (!det_result->name)
      continue;
    int fontw = 0, fonth = 0;
    SDL_Surface *name = sdl_font.GetFontPicture(
        (char *)det_result->name, strlen(det_result->name), 32, &fontw, &fonth);
    if (name) {
      SDL_Rect texture_dimension;
      SDL_Texture *texture = load_texture(name, renderer, &texture_dimension);
      SDL_FreeSurface(name);
      SDL_Rect dst_dimension;
      dst_dimension.x = rect.x;
      dst_dimension.y = rect.y - 18;
      dst_dimension.w = texture_dimension.w;
      dst_dimension.h = texture_dimension.h;
      SDL_RenderCopyEx(renderer, texture, &texture_dimension, &dst_dimension,
                       rotate, NULL, SDL_FLIP_NONE);
      SDL_DestroyTexture(texture);
    }
  }
  return true;
}

} // namespace NPU_UVC_SSD_DEMO
