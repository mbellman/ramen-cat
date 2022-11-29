#version 460 core

uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_colorAndDepth;

#include "utils/conversion.glsl";
#include "utils/skybox.glsl";

void main() {
  // @todo figure out how to calculate direction
  // from camera direction + fragUv
  vec3 position = getWorldPosition(1.0, fragUv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 direction = normalize(position);

  out_colorAndDepth = vec4(getSkyColor(direction), 1.0);
}