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

#ifndef NPU_UVC_PP_OUTPUT_
#define NPU_UVC_PP_OUTPUT_
#include "npu_uvc_shared.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <rknn_runtime.h>

#include "globle.h"

class NPUPostProcessOutput;
typedef bool (*PostDrawFunc)(SDL_Renderer *renderer,
                             const SDL_Rect &render_rect,
                             const SDL_Rect &coor_rect, int rotate,
                             NPUPostProcessOutput *output, void *buffer,
                             Uint32 sdl_fmt);
SDL_Rect transform(const SDL_Rect &src_rect, const SDL_Rect &coor_rect,
                   int rotate);

class NPUPostProcessOutput {
public:
  NPUPostProcessOutput(struct extra_jpeg_data *input,rknn_output *output);
  ~NPUPostProcessOutput() {
    if (pp_output)
      free(pp_output);
  }

  void *pp_output;
  uint32_t count;
  PostDrawFunc pp_func;
  struct npu_widthheight npuwh;
};

class SDLFont {
public:
  SDLFont(SDL_Color forecol, int ptsize);
  ~SDLFont();
  SDL_Surface *DrawString(char *str, int str_length);
  SDL_Surface *GetFontPicture(char *str, int str_length, int bpp, int *w,
                              int *h);
  SDL_Color fore_col;
  SDL_Color back_col;
  int renderstyle;
  enum { RENDER_LATIN1, RENDER_UTF8, RENDER_UNICODE } rendertype;
  int pt_size;
  TTF_Font *font;
};
extern SDL_Color red;
SDL_Texture *load_texture(SDL_Surface *sur, SDL_Renderer *render,
                          SDL_Rect *texture_dimensions);
int draw_rect(SDL_Renderer *renderer, const SDL_Rect *rect, void *buffer,
              Uint32 sdl_fmt);

int draw_points(SDL_Renderer *renderer, const SDL_Point *points, int count, void *buffer,
              Uint32 sdl_fmt);

bool countRectXY(NPUPostProcessOutput *output, float* resultArray, float* lastXY, int src_w, int src_h, int clip_w, int clip_h);

#endif // #ifndef NPU_UVC_PP_OUTPUT_
