#include <string>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "opengl/OpenGLRenderer.h"
#include "performance/benchmark.h"
#include "performance/tools.h"
#include "system/assert.h"
#include "system/console.h"
#include "system/context.h"
#include "system/file.h"
#include "system/flags.h"
#include "system/scene.h"

using namespace Gamma;

#define String(value) std::to_string(value)

static void Gm_DisplayDevtools(GmContext* context) {
  using namespace Gamma;

  auto& renderer = *context->renderer;
  auto& resolution = renderer.getInternalResolution();
  auto& renderStats = renderer.getRenderStats();
  auto& sceneStats = Gm_GetSceneStats(context);
  auto& fpsAverager = context->fpsAverager;
  auto& frameTimeAverager = context->frameTimeAverager;
  auto& commander = context->commander;
  auto& window = context->window;
  auto* font_sm = window.font_sm;
  auto* font_lg = window.font_lg;
  u64 averageFrameTime = frameTimeAverager.average();
  u32 frameTimeBudget = u32(100.0f * (float)averageFrameTime / 16667.0f);

  if (Gm_IsFlagEnabled(GammaFlags::ENABLE_DEV_TOOLS)) {
    // Render system-defined debug messages
    {
      auto fpsLabel = "FPS: "
        + String(fpsAverager.average())
        + ", low "
        + String(fpsAverager.low())
        + " (V-Sync " + (renderStats.isVSynced ? "ON" : "OFF") + ")";

      auto frameTimeLabel = "Frame time: "
        + String(averageFrameTime)
        + "us, high "
        + String(frameTimeAverager.high())
        + " ("
        + String(frameTimeBudget)
        + "%)";

      auto resolutionLabel = "Resolution: " + String(resolution.width) + " x " + String(resolution.height);
      auto vertsLabel = "Verts: " + String(sceneStats.verts);
      auto trisLabel = "Tris: " + String(sceneStats.tris);
      auto totalLightsLabel = "Lights: " + String(sceneStats.totalLights);
      auto totalMeshesLabel = "Meshes: " + String(sceneStats.totalMeshes);
      auto memoryLabel = "GPU Memory: " + String(renderStats.gpuMemoryUsed) + "MB / " + String(renderStats.gpuMemoryTotal) + "MB";

      const Vec3f TEXT_COLOR = Vec3f(1.f);
      const Vec4f BACKGROUND_COLOR = Vec4f(0.5f, 0, 0, 0.5f);

      renderer.renderText(font_sm, fpsLabel.c_str(), 25, 25, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, frameTimeLabel.c_str(), 25, 50, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, resolutionLabel.c_str(), 25, 75, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, vertsLabel.c_str(), 25, 100, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, trisLabel.c_str(), 25, 125, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, totalLightsLabel.c_str(), 25, 150, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, totalMeshesLabel.c_str(), 25, 175, TEXT_COLOR, BACKGROUND_COLOR);
      renderer.renderText(font_sm, memoryLabel.c_str(), 25, 200, TEXT_COLOR, BACKGROUND_COLOR);
    }

    // Render user-defined debug messages
    {
      const Vec3f TEXT_COLOR = Vec3f(1.f);
      const Vec4f BACKGROUND_COLOR = Vec4f(0.0f, 0, 0, 0.8f);

      u8 index = 0;

      for (auto& message : context->debugMessages) {
        renderer.renderText(font_sm, message.c_str(), 25, 250 + index++ * 25, TEXT_COLOR, BACKGROUND_COLOR);
      }
    }

    // Display console messages
    {
      auto* message = Console::getFirstMessage();
      u8 messageIndex = 0;

      // @todo clear messages after a set duration
      while (message != nullptr) {
        auto color = message->warning ? Vec3f(0.8f, 0, 0) : Vec3f(1.f);

        renderer.renderText(font_sm, message->text.c_str(), 25, window.size.height - 150 + (messageIndex++) * 25, color);

        message = message->next;
      }
    }

    // Display dev buffer labels
    {
      if (Gm_IsFlagEnabled(GammaFlags::ENABLE_DEV_BUFFERS)) {
        const auto FG_COLOR = Vec3f(1.f);
        const auto BG_COLOR = Vec4f(0, 0, 0, 0.75f);

        renderer.renderText(font_sm, "Color", u32(window.size.width * 0.55f), u32(window.size.height * 0.035f), FG_COLOR, BG_COLOR);
        renderer.renderText(font_sm, "Depth", u32(window.size.width * 0.657f), u32(window.size.height * 0.035f), FG_COLOR, BG_COLOR);
        renderer.renderText(font_sm, "Normals", u32(window.size.width * 0.765f), u32(window.size.height * 0.035f), FG_COLOR, BG_COLOR);
        renderer.renderText(font_sm, "Material", u32(window.size.width * 0.872f), u32(window.size.height * 0.035f), FG_COLOR, BG_COLOR);
      }
    }
  }

  // Display command line
  {
    if (commander.isOpen()) {
      std::string caret = SDL_GetTicks() % 1000 < 500 ? "_" : "  ";
      std::string command = "> " + commander.getCommand() + caret;
      const Vec3f fgColor = Vec3f(0.0f, 1.0f, 0.0f);
      const Vec4f bgColor = Vec4f(0.0f, 0.0f, 0.0f, 0.8f);

      renderer.renderText(font_lg, command.c_str(), 25, window.size.height - 200, fgColor, bgColor);
    }
  }
}

GmContext* Gm_CreateContext() {
  auto* context = new GmContext();

  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);

  context->window.font_sm = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 16);
  context->window.font_lg = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 22);

  return context;
}

void Gm_OpenWindow(GmContext* context, const char* title, const Gamma::Area<u32>& size) {
  context->window.sdl_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    size.width, size.height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );

  context->window.size = size;
}

void Gm_SetWindowSize(GmContext* context, const Area<u32>& size) {
  context->window.size = size;

  SDL_SetWindowSize(context->window.sdl_window, size.width, size.height);
}

void Gm_SetRenderMode(GmContext* context, GmRenderMode mode) {
  assert(context->window.sdl_window != nullptr, "Attempted to set render mode before calling Gm_OpenWindow()!");

  if (context->renderer != nullptr) {
    context->renderer->destroy();

    delete context->renderer;

    context->renderer = nullptr;
  }

  switch (mode) {
    case GmRenderMode::OPENGL:
      context->renderer = new OpenGLRenderer(context);
      break;
    case GmRenderMode::VULKAN:
      // @todo
      break;
  }

  if (context->renderer != nullptr) {
    context->renderer->init();
  }
}

float Gm_GetDeltaTime(GmContext* context) {
  u32 ticks = SDL_GetTicks();
  float dt = float(ticks - context->lastTick) / 1000.0f;

  context->lastTick = ticks;

  return dt;
}

void Gm_HandleFrameStart(GmContext* context) {
  context->frameStartMicroseconds = Gm_GetMicroseconds();

  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        context->window.closed = true;
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          context->window.size = {
            (u32)event.window.data1,
            (u32)event.window.data2
          };
        }

        break;
      default:
        break;
    }

    if (!context->commander.isOpen()) {
      context->scene.input.handleEvent(event);
    }

    #if GAMMA_DEVELOPER_MODE
      context->commander.input.handleEvent(event);
    #endif
  }

  if (context->lastTick - context->lastWatchedFilesCheckTime > 1000) {
    Gm_HandleWatchedFiles();

    context->lastWatchedFilesCheckTime = context->lastTick;
  }
}

void Gm_RenderScene(GmContext* context) {
  auto& renderer = *context->renderer;

  renderer.render();

  for (auto& [ image, x, y, w, h ] : context->scene.ui.surfaces) {
    renderer.renderSurface(image, x, y, w, h, Vec3f(1.f), Vec4f(0.f));
  }

  for (auto& [ font, text, x, y ] : context->scene.ui.texts) {
    renderer.renderText(font, text.c_str(), x, y, Vec3f(1.f), Vec4f(0.f));
  }

  #if GAMMA_DEVELOPER_MODE
    Gm_DisplayDevtools(context);
  #endif

  renderer.present();
}

void Gm_HandleFrameEnd(GmContext* context) {
  using namespace Gamma;

  u64 frameTimeInMicroseconds = Gm_GetMicroseconds() - context->frameStartMicroseconds;
  u32 fps = (u32)(1000000.0f / (float)frameTimeInMicroseconds);

  context->fpsAverager.add(fps);
  context->frameTimeAverager.add(frameTimeInMicroseconds);
  context->contextTime += frameTimeInMicroseconds / 1000000.0f;

  context->scene.frame++;
  context->scene.input.resetPerFrameState();
  context->scene.ui.surfaces.clear();
  context->scene.ui.texts.clear();

  context->debugMessages.clear();

  Gm_SavePreviousFlags();
}

void Gm_DestroyContext(GmContext* context) {
  // @todo clear scene

  IMG_Quit();

  TTF_CloseFont(context->window.font_sm);
  TTF_CloseFont(context->window.font_lg);
  TTF_Quit();

  SDL_DestroyWindow(context->window.sdl_window);
  SDL_Quit();
}

bool Gm_IsWindowFocused() {
  return SDL_GetRelativeMouseMode();
}

void Gm_FocusWindow() {
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Gm_UnfocusWindow() {
  SDL_SetRelativeMouseMode(SDL_FALSE);
}