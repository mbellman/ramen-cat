#pragma once

#include "system/traits.h"
#include "system/type_aliases.h"

struct GmContext;
struct SDL_Surface;

namespace Gamma {
  struct Mesh;
  struct Light;

  struct FrameFlags {
    /**
     * Controls whether we want to use an identical
     * previous-frame camera view matrix to the current
     * frame for temporal sampling. The net result is
     * that points will not be reprojected, but sampled
     * at the same position from the previous frame buffer.
     */
    bool useStableTemporalSampling = false;
  };

  struct RenderStats {
    u32 gpuMemoryTotal = 0;
    u32 gpuMemoryUsed = 0;
    bool isVSynced = false;
  };

  class AbstractRenderer : public Initable, public Renderable, public Destroyable {
  public:
    FrameFlags frameFlags;

    AbstractRenderer(GmContext* gmContext): gmContext(gmContext) {};
    virtual ~AbstractRenderer() {};

    virtual void createMesh(Mesh* mesh) {};
    virtual void createShadowMap(Light* light) {};
    virtual void destroyMesh(Mesh* mesh) {};
    virtual void destroyShadowMap(Light* light) {};
    virtual void destroyProbe(const std::string& name) {};

    virtual Area<u32>& getInternalResolution() final {
      return internalResolution;
    }

    virtual const RenderStats& getRenderStats() = 0;
    virtual void present() {};
    virtual void renderSurface(SDL_Surface* surface, u32 x, u32 y, u32 w, u32 h, const Vec3f& color, const Vec4f& background) {};
    virtual void renderText(TTF_Font* font, const char* message, u32 x, u32 y, const Vec3f& color = Vec3f(1.0f), const Vec4f& background = Vec4f(0.0f)) {};
    virtual void resetShadowMaps() {};

  protected:
    GmContext* gmContext = nullptr;
    Area<u32> internalResolution = { 1920, 1080 };
    RenderStats stats = { 0, 0, false };
  };
}