#pragma once

#include "math/plane.h"
#include "performance/tools.h"
#include "system/AbstractRenderer.h"
#include "system/Commander.h"
#include "system/entities.h"
#include "system/macros.h"
#include "system/scene.h"
#include "system/traits.h"
#include "system/type_aliases.h"

#define _ctx GmContext* context

enum GmRenderMode {
  OPENGL,
  VULKAN
};

struct GmContext {
  GmScene scene;
  Gamma::AbstractRenderer* renderer = nullptr;
  u32 lastTick = 0;
  u64 frameStartMicroseconds = 0;
  u32 lastWatchedFilesCheckTime = 0;
  // @todo debug-mode only
  Gamma::Averager<5, u32> fpsAverager;
  Gamma::Averager<5, u64> frameTimeAverager;
  Gamma::Commander commander;
  std::vector<std::string> debugMessages;

  struct GmWindow {
    bool closed = false;
    TTF_Font* font_sm = nullptr;
    TTF_Font* font_lg = nullptr;
    SDL_Window* sdl_window = nullptr;
    Gamma::Area<u32> size;
  } window;
};

GmContext* Gm_CreateContext();
void Gm_OpenWindow(GmContext* context, const char* title, const Gamma::Area<u32>& size);
void Gm_SetRenderMode(GmContext* context, GmRenderMode mode);
float Gm_GetDeltaTime(GmContext* context);
void Gm_LogFrameStart(GmContext* context);
void Gm_HandleEvents(GmContext* context);
void Gm_RenderScene(GmContext* context);
void Gm_LogFrameEnd(GmContext* context);
void Gm_DestroyContext(GmContext* context);