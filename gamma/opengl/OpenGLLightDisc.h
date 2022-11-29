#pragma once

#include <vector>

#include "math/plane.h"
#include "system/entities.h"
#include "system/traits.h"
#include "system/type_aliases.h"

namespace Gamma {
  struct Disc {
    Vec2f offset;
    Vec2f scale;
    Light light;
  };

  class OpenGLLightDisc : public Initable, public Destroyable {
  public:
    virtual void init() override;
    virtual void destroy() override;
    void draw(const Light& light, const Area<u32>& resolution, const Camera& camera);
    void draw(const std::vector<Light*>& lights, const Area<u32>& resolution, const Camera& camera);

  private:
    GLuint vao;
    /**
     * Buffers for light disc attributes.
     *
     * [0] Vertex
     * [2] Disc instances (scale, offset, light)
     */
    GLuint buffers[2];

    void configureDisc(Disc& disc, const Light& light, const Matrix4f& matProjection, const Matrix4f& matView, float resolutionAspectRatio);
  };
}