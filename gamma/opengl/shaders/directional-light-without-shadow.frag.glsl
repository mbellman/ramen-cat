#version 460 core

#define MAX_DIRECTIONAL_LIGHTS 10

struct DirectionalLight {
  vec3 color;
  float power;
  vec3 direction;
};

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform DirectionalLight lights[10];

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_colorAndDepth;

#include "utils/conversion.glsl";

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec4 frag_normal_and_emissivity = texture(texNormalAndEmissivity, fragUv);
  vec3 position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
  vec3 normal = frag_normal_and_emissivity.xyz;
  vec3 color = frag_color_and_depth.rgb;
  float emissivity = frag_normal_and_emissivity.w;

  // @todo store roughness in a third 'material' G-Buffer channel
  const float roughness = 0.6;

  vec3 accumulatedColor = vec3(0.0);

  for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; i++) {
    DirectionalLight light = lights[i];

    #include "inline/directional-light.glsl";

    accumulatedColor += illuminated_color + fresnel_term;
  }

  out_colorAndDepth = vec4(accumulatedColor * (1.0 - emissivity), frag_color_and_depth.w);
}