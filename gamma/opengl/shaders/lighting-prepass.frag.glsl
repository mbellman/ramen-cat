#version 460 core

#define USE_INDIRECT_SKY_LIGHT 1

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndMaterial;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 atmosphereColor;
uniform float altitude;

in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/skybox.glsl";
#include "utils/conversion.glsl";

const vec3 sky_sample_offsets[] = {
  vec3(1, 0, 0),
  vec3(-1, 0, 0),
  vec3(0, 1, 0),
  vec3(0, -1, 0),
  vec3(0, 0, 1),
  vec3(0, 0, -1)
};

vec3 getIndirectSkyLightContribution(vec3 fragment_position, vec3 fragment_normal, float roughness) {
  // @todo pass in as a uniform
  const float indirect_sky_light_intensity = 0.5;
  vec3 diffuse_contribution = vec3(0);

  for (int i = 0; i < 6; i++) {
    vec3 sky_direction = sky_sample_offsets[i];

    diffuse_contribution += getSkyColor(sky_direction, sunDirection, sunColor, atmosphereColor, altitude).rgb * indirect_sky_light_intensity;
  }

  diffuse_contribution /= 6.0;

  vec3 camera_to_fragment = fragment_position - cameraPosition;
  vec3 reflection_vector = normalize(reflect(camera_to_fragment, fragment_normal));
  vec3 specular_contribution = getSkyColor(reflection_vector, sunDirection, sunColor, atmosphereColor, altitude).rgb;

  return mix(diffuse_contribution, specular_contribution, pow(1.0 - roughness, 3));
}

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec4 frag_normal_and_material = texture(texNormalAndMaterial, fragUv);
  vec3 frag_color = frag_color_and_depth.rgb;
  float frag_depth = frag_color_and_depth.w;
  vec3 fragment_position = getWorldPosition(frag_depth, fragUv, matInverseProjection, matInverseView);
  vec3 frag_normal = frag_normal_and_material.xyz;
  vec3 out_color = vec3(0.0);

  // @todo refactor
  float material = frag_normal_and_material.w;
  float emissivity = floor(material) / 10.0;
  float roughness = fract(material);

  #if USE_INDIRECT_SKY_LIGHT == 1
    out_color += frag_color * getIndirectSkyLightContribution(fragment_position, frag_normal, roughness);
  #endif

  vec3 fragment_to_camera = normalize(cameraPosition - fragment_position);
  float fresnel_factor = pow(1.0 - max(dot(frag_normal, fragment_to_camera), 0.0), 5);

  out_color += frag_color * emissivity;
  out_color += vec3(1) * fresnel_factor * 0.1;

  out_color_and_depth = vec4(out_color, frag_depth);
}