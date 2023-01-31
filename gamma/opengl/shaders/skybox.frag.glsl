#version 460 core

uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform float time;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 atmosphereColor;
uniform float altitude;

// @temporary
uniform sampler2D texClouds;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/conversion.glsl";
#include "utils/skybox.glsl";

#define PI 3.141592
#define TAU PI * 2.0

void main() {
  // @todo figure out how to calculate direction
  // from camera direction + fragUv
  vec3 position = getWorldPosition(1.0, fragUv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 direction = normalize(position);
  vec3 sky_color = getSkyColor(direction, sunDirection, sunColor, atmosphereColor, altitude).rgb;

  vec2 cloudsUv = vec2(
    -(atan(direction.z, direction.x) + PI) / TAU + time * CLOUD_MOVEMENT_RATE,
    -direction.y - 0.5
  );

  vec4 clouds = texture(texClouds, cloudsUv);
  vec3 clouds_color = mix(clouds.rgb, sky_color, 0.3) * clouds.a;

  // @hack avoid clouds texture distortion near vec3(0, 1, 0)
  // @todo make this configurable, or determine a better way to avoid distortion
  if (abs(direction.y) > 0.7) {
    clouds_color = vec3(0);
  }

  out_color_and_depth = vec4(sky_color + clouds_color, 1.0);
}