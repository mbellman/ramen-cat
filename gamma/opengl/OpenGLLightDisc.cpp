#include <cmath>

#include "glew.h"

#include "opengl/OpenGLLightDisc.h"
#include "math/constants.h"
#include "math/matrix.h"
#include "system/camera.h"
#include "system/console.h"

namespace Gamma {
  constexpr static u32 DISC_SLICES = 16;

  enum GLBuffer {
    VERTEX,
    DISC
  };

  enum GLAttribute {
    VERTEX_POSITION,
    DISC_OFFSET,
    DISC_SCALE,
    DISC_LIGHT_POSITION,
    DISC_LIGHT_RADIUS,
    DISC_LIGHT_COLOR,
    DISC_LIGHT_POWER,
    DISC_LIGHT_DIRECTION,
    DISC_LIGHT_FOV
  };

  static inline Matrix4f getLightViewMatrix(const Camera& camera) {
    Orientation viewOrientation = camera.orientation.invert();

    viewOrientation.roll *= -1.f;  // @hack

    return (
      Matrix4f::rotation(viewOrientation) *
      Matrix4f::translation(camera.position.invert())
    );
  }

  static inline Matrix4f getLightProjectionMatrix(const Area<u32>& resolution, const float fov) {
    const static float near = 1.f;
    const static float far = 10000.f;

    return Matrix4f::glPerspective(resolution, fov, near, far);
  }

  void OpenGLLightDisc::init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(2, &buffers[0]);
    glBindVertexArray(vao);

    // Create the vertices for each slice of the disc
    Vec2f vertexPositions[DISC_SLICES * 3];

    for (u32 i = 0; i < DISC_SLICES; i++) {
      constexpr static float sliceAngle = 360.f / (float)DISC_SLICES;
      u32 index = i * 3;

      // Add center vertex
      vertexPositions[index] = Vec2f(0.f);

      // Add corners
      const float a1 = i * sliceAngle * DEGREES_TO_RADIANS;
      const float a2 = (i + 1) * sliceAngle * DEGREES_TO_RADIANS;

      vertexPositions[index + 1] = Vec2f(sinf(a1), cosf(a1));
      vertexPositions[index + 2] = Vec2f(sinf(a2), cosf(a2));
    }

    // Buffer disc vertices
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2f) * DISC_SLICES * 3, vertexPositions, GL_STATIC_DRAW);

    // Define disc vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::VERTEX]);

    glEnableVertexAttribArray(GLAttribute::VERTEX_POSITION);
    glVertexAttribPointer(GLAttribute::VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2f), (void*)0);

    // Define disc instance attributes
    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::DISC]);

    glEnableVertexAttribArray(GLAttribute::DISC_OFFSET);
    glVertexAttribPointer(GLAttribute::DISC_OFFSET, 2, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)offsetof(Disc, offset));
    glVertexAttribDivisor(GLAttribute::DISC_OFFSET, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_SCALE);
    glVertexAttribPointer(GLAttribute::DISC_SCALE, 2, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)offsetof(Disc, scale));
    glVertexAttribDivisor(GLAttribute::DISC_SCALE, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_POSITION);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, position)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_POSITION, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_RADIUS);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_RADIUS, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, radius)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_RADIUS, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_COLOR);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, color)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_COLOR, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_POWER);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_POWER, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, power)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_POWER, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_DIRECTION);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_DIRECTION, 3, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, direction)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_DIRECTION, 1);

    glEnableVertexAttribArray(GLAttribute::DISC_LIGHT_FOV);
    glVertexAttribPointer(GLAttribute::DISC_LIGHT_FOV, 1, GL_FLOAT, GL_FALSE, sizeof(Disc), (void*)(offsetof(Disc, light) + offsetof(Light, fov)));
    glVertexAttribDivisor(GLAttribute::DISC_LIGHT_FOV, 1);
  }

  void OpenGLLightDisc::destroy() {
    // @todo
  }

  void OpenGLLightDisc::configureDisc(Disc& disc, const Light& light, const Matrix4f& matProjection, const Matrix4f& matView, float resolutionAspectRatio) {
    Vec3f localLightPosition = (matView * light.position).toVec3f();

    disc.light = light;

    if (localLightPosition.z > 0.1f) {
      // Light source in front of the camera
      Vec3f screenLightPosition = (matProjection * localLightPosition).toVec3f() / localLightPosition.z;

      disc.offset = Vec2f(screenLightPosition.x, screenLightPosition.y);
      // @todo use 1 + log(light.power) or similar for scaling term
      disc.scale.x = 1.5f * light.radius / localLightPosition.z;
      disc.scale.y = 1.5f * light.radius / localLightPosition.z * resolutionAspectRatio;
    } else {
      // Light source behind the camera; scale to cover
      // screen when within range, and scale to 0 when
      // out of range
      float scale = localLightPosition.magnitude() < light.radius ? 2.f : 0.f;

      disc.offset = Vec2f(0.f);
      disc.scale = Vec2f(scale);
    }
  }

  void OpenGLLightDisc::draw(const Light& light, const Area<u32>& resolution, const Camera& camera) {
    Disc discs[1];
    auto& disc = discs[0];
    float aspectRatio = (float)resolution.width / (float)resolution.height;
    Matrix4f matProjection = getLightProjectionMatrix(resolution, camera.fov);
    Matrix4f matView = getLightViewMatrix(camera);

    configureDisc(disc, light, matProjection, matView, aspectRatio);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::DISC]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Disc), discs, GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, DISC_SLICES * 3);
  }

  void OpenGLLightDisc::draw(const std::vector<Light*>& lights, const Area<u32>& resolution, const Camera& camera) {
    // @todo avoid reallocating/freeing the disc array on each draw
    Disc* discs = new Disc[lights.size()];
    float aspectRatio = (float)resolution.width / (float)resolution.height;
    Matrix4f matProjection = getLightProjectionMatrix(resolution, camera.fov);
    Matrix4f matView = getLightViewMatrix(camera);

    for (u32 i = 0; i < lights.size(); i++) {
      auto& light = *lights[i];
      auto& disc = discs[i];

      configureDisc(disc, light, matProjection, matView, aspectRatio);
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers[GLBuffer::DISC]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Disc) * lights.size(), discs, GL_DYNAMIC_DRAW);

    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, DISC_SLICES * 3, lights.size());

    delete[] discs;
  }
}