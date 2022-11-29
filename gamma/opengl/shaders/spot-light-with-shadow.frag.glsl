#version 460 core

#define USE_VARIABLE_PENUMBRA_SIZE 1

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
uniform sampler2D texShadowMap;
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform mat4 lightMatrix;
uniform float time;

// @todo pass in as a uniform
const float indirect_light_factor = 0.01;

noperspective in vec2 fragUv;
flat in Light light;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/gl.glsl";
#include "utils/conversion.glsl";
#include "utils/random.glsl";
#include "utils/helpers.glsl";

vec2 rotatedVogelDisc(int samples, int index) {
  float rotation = noise(fract(1.0 + time)) * 3.141592;
  float theta = 2.4 * index + rotation;
  float radius = sqrt(float(index) + 0.5) / sqrt(float(samples));

  return radius * vec2(cos(theta), sin(theta));
}

float getLightFactor(vec3 position, float incidence, float light_distance) {
  // @hack invert Z
  vec4 transform = lightMatrix * glVec4(position);

  transform.xyz /= transform.w;
  transform.xyz *= 0.5;
  transform.xyz += 0.5;

  #if USE_VARIABLE_PENUMBRA_SIZE == 1
    const float max_spread = 200.0;
    float spread = 1.0 + pow(light_distance / light.radius, 2) * max_spread;
  #else
    float spread = 3.0;
  #endif

  const int TOTAL_SAMPLES = 12;
  const vec2 shadow_map_texel_size = 1.0 / vec2(1024.0);
  float bias = mix(0.001, 0.0002, saturate(light_distance / 100.0));
  float factor = 0.0;

  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    vec2 texel_offset = spread * rotatedVogelDisc(TOTAL_SAMPLES, i) * shadow_map_texel_size;
    vec2 texel_coords = transform.xy + texel_offset;
    float occluder_distance = texture(texShadowMap, texel_coords).r;

    if (occluder_distance > transform.z - bias) {
      factor += 1.0;
    }
  }

  return factor / float(TOTAL_SAMPLES);
}

void main() {
  #include "inline/spot-light.glsl";

  float light_factor = getLightFactor(position, incidence, light_distance);

  out_color_and_depth = vec4(illuminated_color * light_factor, frag_color_and_depth.w);
}