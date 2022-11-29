#version 460 core

#define USE_VARIABLE_PENUMBRA_SIZE 1

struct DirectionalLight {
  vec3 color;
  float power;
  vec3 direction;
};

struct Cascade {
  int index;
  mat4 matrix;
  float bias;
  float spread_factor;
  float occluder_sweep_radius;
};

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform sampler2D texShadowMaps[3];
uniform vec3 cameraPosition;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform mat4 lightMatrices[3];
uniform DirectionalLight light;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

const float cascade_depth_1 = 200.0;
const float cascade_depth_2 = 600.0;

#include "utils/gl.glsl";
#include "utils/conversion.glsl";
#include "utils/random.glsl";

Cascade getCascadeByDepth(float linearized_depth) {
  if (linearized_depth < cascade_depth_1) {
    return Cascade(0, lightMatrices[0], 0.0002, 1000.0, 20.0);
  } else if (linearized_depth < cascade_depth_2) {
    return Cascade(1, lightMatrices[1], 0.0005, 750.0, 5.0);
  } else {
    return Cascade(2, lightMatrices[2], 0.0005, 500.0, 3.0);
  }
}

vec2 rotatedVogelDisc(int samples, int index) {
  float rotation = noise(1.0) * 3.141592;
  float theta = 2.4 * index + rotation;
  float radius = sqrt(float(index) + 0.5) / sqrt(float(samples));

  return radius * vec2(cos(theta), sin(theta));
}

float getClosestOccluder(sampler2D shadow_map, vec2 shadow_map_texel_size, vec4 transform, float occluder_sweep_radius) {
  // @todo move to utils/offsets.glsl
  const vec2 offsets[9] = {
    vec2(0.0),
    vec2(-1.0, 0.0),
    vec2(-1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, -1.0),
    vec2(0.0, -1.0),
    vec2(-1.0, 1.0)
  };

  float closest_occluder = transform.z;

  for (int i = 0; i < 9; i++) {
    vec2 texel_coords = transform.xy + occluder_sweep_radius * shadow_map_texel_size * offsets[i];
    float shadow_map_depth = texture(shadow_map, texel_coords).r;

    if (shadow_map_depth < closest_occluder) {
      closest_occluder = shadow_map_depth;
    }
  }

  return closest_occluder;
}

float getLightIntensity(Cascade cascade, vec4 transform) {
  if (transform.z > 0.999) {
    // If the position-to-light space transform depth
    // is out of range, we've sampled outside the
    // shadow map and can just render the fragment
    // with full illumination.
    return 1.0;
  }

  vec2 shadow_map_texel_size = 1.0 / textureSize(texShadowMaps[cascade.index], 0);

  #if USE_VARIABLE_PENUMBRA_SIZE == 1
    const float MAX_SPREAD = 15.0;

    float closest_occluder = getClosestOccluder(texShadowMaps[cascade.index], shadow_map_texel_size, transform, cascade.occluder_sweep_radius);
    float spread = min(MAX_SPREAD, 1.0 + cascade.spread_factor * pow(distance(transform.z, closest_occluder), 2));
  #else
    float spread = cascade.spread_factor / 500.0;
  #endif

  const int TOTAL_SAMPLES = 16;
  float light_intensity = 0.0;

  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    vec2 texel_offset = spread * rotatedVogelDisc(TOTAL_SAMPLES, i) * shadow_map_texel_size;
    vec2 texel_coords = transform.xy + texel_offset;
    float shadow_map_depth = texture(texShadowMaps[cascade.index], texel_coords).r;

    if (shadow_map_depth > transform.z - (cascade.bias + spread * 0.0005)) {
      light_intensity += 1.0;
    }
  }

  return light_intensity / float(TOTAL_SAMPLES);
}

vec4 getLightSpaceTransform(mat4 matLight, vec3 position) {
  // @hack invert Z
  vec4 transform = matLight * glVec4(position);

  transform.xyz /= transform.w;
  transform.xyz *= 0.5;
  transform.xyz += 0.5;

  return transform;
}

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec4 frag_normal_and_emissivity = texture(texNormalAndEmissivity, fragUv);
  vec3 color = frag_color_and_depth.rgb;
  vec3 position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
  vec3 normal = frag_normal_and_emissivity.xyz;
  float emissivity = frag_normal_and_emissivity.w;

  // @todo store roughness in a third 'material' G-Buffer channel
  const float roughness = 0.6;

  #include "inline/directional-light.glsl";

  Cascade cascade = getCascadeByDepth(getLinearizedDepth(frag_color_and_depth.w));
  vec4 light_space_transform = getLightSpaceTransform(lightMatrices[cascade.index], position);
  float light_intensity = getLightIntensity(cascade, light_space_transform);

  out_color_and_depth = vec4(illuminated_color * light_intensity * (1.0 - emissivity) + fresnel_term, frag_color_and_depth.w);
}