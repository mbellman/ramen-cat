#version 460 core

#define MAX_DIRECTIONAL_LIGHTS 10

struct DirectionalLight {
  vec3 color;
  float power;
  vec3 direction;
};

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndMaterial;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform DirectionalLight lights[10];

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_colorAndDepth;

#include "utils/conversion.glsl";

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec4 frag_normal_and_material = texture(texNormalAndMaterial, fragUv);
  vec3 position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
  vec3 normal = frag_normal_and_material.xyz;
  vec3 color = frag_color_and_depth.rgb;

  // @todo refactor
  float material = frag_normal_and_material.w;
  float emissivity = floor(material) / 10.0;
  float roughness = fract(material);

  vec3 accumulatedColor = vec3(0.0);

  for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
    DirectionalLight light = lights[i];

    #include "inline/directional-light.glsl";

    accumulatedColor += illuminated_color;
  }

  out_colorAndDepth = vec4(accumulatedColor * (1.0 - min(1.0, emissivity)), frag_color_and_depth.w);
}