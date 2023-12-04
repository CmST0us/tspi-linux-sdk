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

#ifndef NPU_UVC_ROCKX_DRAW_
#define NPU_UVC_ROCKX_DRAW_

#include <SDL2/SDL.h>

class NPUPostProcessOutput;
namespace NPU_UVC_ROCKX_DEMO {

bool RockxFaceGenderAgeDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
                            const SDL_Rect &coor_rect, int rotate,
                            NPUPostProcessOutput *output, void *buffer,
                            Uint32 sdl_fmt);
bool RockxFaceDetectDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
                         const SDL_Rect &coor_rect, int rotate,
                         NPUPostProcessOutput *output, void *buffer,
                         Uint32 sdl_fmt);
bool RockxFaceLandMarkDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
                         const SDL_Rect &coor_rect, int rotate,
                         NPUPostProcessOutput *output, void *buffer,
                         Uint32 sdl_fmt);

} // namespace NPU_UVC_ROCKX_DEMO

#endif // #ifndef NPU_UVC_ROCKX_DRAW_