#include "SDL.h"

#include "system/immediate_ui.h"

SDL_Surface* Gm_CreateRectangle(u32 width, u32 height, u32 color) {
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    auto* surface = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
  #else
    auto* surface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  #endif

  auto rgba = SDL_MapRGBA(surface->format, (color & 0xFF000000) >> 24, (color & 0x00FF0000) >> 16, (color & 0x0000FF00) >> 8, color & 0x000000FF);

  SDL_FillRect(surface, 0, rgba);

  return surface;
}