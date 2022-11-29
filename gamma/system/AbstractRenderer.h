#pragma once

#include "system/traits.h"
#include "system/type_aliases.h"

struct GmContext;

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
    u32 gpuMemoryTotal;
    u32 gpuMemoryUsed;
    bool isVSynced;
  };

  class AbstractRenderer : public Initable, public Renderable, public Destroyable {
  public:
    FrameFlags frameFlags;

    AbstractRenderer(GmContext* gmContext): gmContext(gmContext) {};
    virtual ~AbstractRenderer() {};

    virtual void createMesh(const Mesh* mesh) {};
    virtual void createShadowMap(const Light* light) {};
    virtual void destroyMesh(const Mesh* mesh) {};
    virtual void destroyShadowMap(const Light* light) {};

    virtual Area<u32>& getInternalResolution() final {
      return internalResolution;
    }

    virtual const RenderStats& getRenderStats() = 0;
    virtual void present() {};
    virtual void renderText(TTF_Font* font, const char* message, u32 x, u32 y, const Vec3f& color = Vec3f(1.0f), const Vec4f& background = Vec4f(0.0f)) {};
    virtual void resetShadowMaps() {};

  protected:
    GmContext* gmContext = nullptr;
    Area<u32> internalResolution = { 1920, 1080 };
    RenderStats stats;
  };
}