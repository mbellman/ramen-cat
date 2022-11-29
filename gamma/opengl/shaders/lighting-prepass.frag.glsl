#version 460 core

#define USE_INDIRECT_SKY_LIGHT 1

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;

in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/skybox.glsl";

const vec3 sky_sample_offsets[] = {
  vec3(1, 0, 0),
  vec3(-1, 0, 0),
  vec3(0, 1, 0),
  vec3(0, -1, 0),
  vec3(0, 0, 1),
  vec3(0, 0, -1)
};

vec3 getIndirectSkyLightContribution(vec3 fragment_normal) {
  // @todo pass in as a uniform
  const float indirect_sky_light_intensity = 0.5;
  vec3 contribution = vec3(0);

  for (int i = 0; i < 6; i++) {
    // @todo use roughness to determine sample offset range
    vec3 direction = normalize(0.2 * fragment_normal + sky_sample_offsets[i]);

    contribution += getSkyColor(direction) * indirect_sky_light_intensity;
  }

  return contribution / 6.0;
}

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec3 out_color = vec3(0.0);

  #if USE_INDIRECT_SKY_LIGHT == 1
    vec3 frag_normal = texture(texNormalAndEmissivity, fragUv).xyz;

    out_color += frag_color_and_depth.rgb * getIndirectSkyLightContribution(frag_normal);
  #endif

  out_color_and_depth = vec4(out_color, frag_color_and_depth.w);
}