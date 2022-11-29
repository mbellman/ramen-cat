#version 460 core

struct Light {
  vec3 position;
  float radius;
  vec3 color;
  float power;
  vec3 direction;
  float fov;
};

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;

// @todo pass in as a uniform
const float indirect_light_factor = 0.01;

noperspective in vec2 fragUv;
flat in Light light;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/conversion.glsl";

void main() {
  #include "inline/spot-light.glsl";

  out_color_and_depth = vec4(illuminated_color, frag_color_and_depth.w);
}