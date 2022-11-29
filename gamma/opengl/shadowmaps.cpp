#include <algorithm>
#include <cmath>

#include "math/constants.h"
#include "opengl/shadowmaps.h"
#include "system/console.h"
#include "system/flags.h"

#include "glew.h"

namespace Gamma {
  // { near, far }
  const static float cascadeDepthRanges[3][2] = {
    { 1.0f, 200.0f },
    { 200.0f, 600.0f },
    { 600.0f, 1250.0f }
  };

  /**
   * OpenGLDirectionalShadowMap
   * --------------------------
   */
  OpenGLDirectionalShadowMap::OpenGLDirectionalShadowMap(const Light* light) {
    this->light = light;

    buffer.init();
    buffer.setSize({ 2048, 2048 });
    buffer.addColorAttachment(ColorFormat::R, 3);  // Cascade 0 (GL_TEXTURE3)
    buffer.addColorAttachment(ColorFormat::R, 4);  // Cascade 1 (GL_TEXTURE4)
    buffer.addColorAttachment(ColorFormat::R, 5);  // Cascade 2 (GL_TEXTURE5)
    buffer.addDepthAttachment();
    buffer.bindColorAttachments();

    #if GAMMA_DEVELOPER_MODE
      Console::log("[Gamma] OpenGLDirectionalShadowMap created");
    #endif
  }

  /**
   * OpenGLPointShadowMap
   * --------------------
   */
  OpenGLPointShadowMap::OpenGLPointShadowMap(const Light* light) {
    this->light = light;

    buffer.init();
    buffer.setSize({ 1024, 1024 });
    buffer.addDepthAttachment(3);  // Depth (GL_TEXTURE3)

    #if GAMMA_DEVELOPER_MODE
      Console::log("[Gamma] OpenGLPointShadowMap created");
    #endif
  }

  /**
   * OpenGLSpotShadowMap
   * -------------------
   */
  OpenGLSpotShadowMap::OpenGLSpotShadowMap(const Light* light) {
    this->light = light;

    buffer.init();
    buffer.setSize({ 1024, 1024 });
    buffer.addColorAttachment(ColorFormat::R, 3);  // Depth (GL_TEXTURE3)
    buffer.addDepthAttachment();
    buffer.bindColorAttachments();

    #if GAMMA_DEVELOPER_MODE
      Console::log("[Gamma] OpenGLSpotShadowMap created");
    #endif
  }

  /**
   * Gm_CreateCascadedLightViewProjectionMatrixGL
   * --------------------------------------------
   *
   * Adapted from https://alextardif.com/shadowmapping.html
   */
  Matrix4f Gm_CreateCascadedLightViewProjectionMatrixGL(u8 cascade, const Vec3f& lightDirection, const Camera& camera) {
    // Determine the near and far ranges of the cascade volume
    float near = cascadeDepthRanges[cascade][0];
    float far = cascadeDepthRanges[cascade][1];

    // Define clip space camera frustum
    Vec3f corners[] = {
      Vec3f(-1.0f, 1.0f, -1.0f),   // Near plane, top left
      Vec3f(1.0f, 1.0f, -1.0f),    // Near plane, top right
      Vec3f(-1.0f, -1.0f, -1.0f),  // Near plane, bottom left
      Vec3f(1.0f, -1.0f, -1.0f),   // Near plane, bottom right

      Vec3f(-1.0f, 1.0f, 1.0f),    // Far plane, top left
      Vec3f(1.0f, 1.0f, 1.0f),     // Far plane, top right
      Vec3f(-1.0f, -1.0f, 1.0f),   // Far plane, bottom left
      Vec3f(1.0f, -1.0f, 1.0f)     // Far plane, bottom right
    };

    // Transform clip space camera frustum into world space
    Matrix4f cameraView = (
      Matrix4f::rotation(camera.orientation) *
      Matrix4f::translation(camera.position.invert().gl())
    );

    // @todo pass internalResolution instead of { 1920, 1080 }
    Matrix4f cameraProjection = Matrix4f::glPerspective({ 1920, 1080 }, camera.fov, near, far);
    Matrix4f cameraViewProjection = cameraProjection * cameraView;
    Matrix4f inverseCameraViewProjection = cameraViewProjection.inverse();

    for (u32 i = 0; i < 8; i++) {
      corners[i] = (inverseCameraViewProjection * corners[i]).homogenize();
      corners[i].z *= -1.0f;
    }

    // Calculate world space frustum center/centroid
    Vec3f frustumCenter;

    for (u32 i = 0; i < 8; i++) {
      frustumCenter += corners[i];
    }

    frustumCenter /= 8.0f;

    // Calculate the radius of a sphere encapsulating the frustum
    float radius = 0.0f;

    for (u32 i = 0; i < 8; i++) {
      radius = std::max(radius, (frustumCenter - corners[i]).magnitude());
    }

    // Calculate the ideal frustum center, 'snapped' to the shadow map texel
    // grid to avoid warbling and other distortions when moving the camera
    float texelsPerUnit = 2048.0f / (radius * 2.0f);

    // Determine the top (up) vector for the lookAt matrix
    bool isVerticalFacingLight = lightDirection == Vec3f(0, 1, 0) || lightDirection == Vec3f(0, -1, 0);
    Vec3f topVector = isVerticalFacingLight ? Vec3f(0, 0, 1) : Vec3f(0, 1, 0);

    Matrix4f texelLookAt = Matrix4f::lookAt(Vec3f(0.0f), lightDirection.invert(), topVector);
    Matrix4f texelScale = Matrix4f::scale(texelsPerUnit);
    Matrix4f texelMatrix = texelScale * texelLookAt;

    // Align the frustum center in texel space, and then
    // restore that to its world space coordinates
    frustumCenter = (texelMatrix * frustumCenter).homogenize();
    frustumCenter.x = floorf(frustumCenter.x);
    frustumCenter.y = floorf(frustumCenter.y);
    frustumCenter = (texelMatrix.inverse() * frustumCenter).homogenize();

    // Compute final light view matrix for rendering the shadow map
    Matrix4f matProjection = Matrix4f::orthographic(radius, -radius, -radius, radius, -radius - 1000.0f, radius);
    Matrix4f matView = Matrix4f::lookAt(frustumCenter.gl(), lightDirection.invert().gl(), topVector);

    return (matProjection * matView).transpose();
  }
}