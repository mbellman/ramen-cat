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
uniform sampler2D texNormalAndMaterial;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;

noperspective in vec2 fragUv;
flat in Light light;
flat in vec2 center;
in float intensity;

layout (location = 0) out vec4 out_colorAndDepth;

#include "utils/conversion.glsl";

void main() {
  #include "inline/point-light.glsl";

  // @todo cleanup
  float radius = length(fragUv - center) * (light_distance_from_camera * 0.0005);
  float glow_factor = clamp(pow(1.0 - radius, 30), 0.0, 1.0);

  if (light_distance_from_camera < light.radius * 3.0) {
    glow_factor *= pow(light_distance_from_camera / (light.radius * 3.0), 3);
  }

  out_colorAndDepth = vec4(illuminated_color + light.color * glow_factor, frag_color_and_depth.w);
}