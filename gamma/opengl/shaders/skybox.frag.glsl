#version 460 core

uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;

// @temporary
uniform sampler2D texClouds;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_colorAndDepth;

#include "utils/conversion.glsl";
#include "utils/skybox.glsl";

#define PI 3.141592
#define TAU PI * 2.0

void main() {
  // @todo figure out how to calculate direction
  // from camera direction + fragUv
  vec3 position = getWorldPosition(1.0, fragUv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 direction = normalize(position);
  vec3 sky_color = getSkyColor(direction).rgb;

  vec2 cloudsUv = vec2(
    -(atan(direction.z, direction.x) + PI) / TAU,
    -direction.y - 0.5
  );

  vec4 clouds = texture(texClouds, cloudsUv);
  vec3 clouds_color = mix(clouds.rgb, sky_color, 0.3) * clouds.a;

  out_colorAndDepth = vec4(sky_color + clouds_color, 1.0);
}