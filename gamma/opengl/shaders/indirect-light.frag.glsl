#version 460 core

#define USE_SCREEN_SPACE_AMBIENT_OCCLUSION 1
#define USE_SCREEN_SPACE_GLOBAL_ILLUMINATION 1
#define USE_DENOISING 1

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform sampler2D texIndirectLightT1;
uniform vec3 cameraPosition;
uniform mat4 matView;
uniform mat4 matInverseView;
uniform mat4 matProjection;
uniform mat4 matInverseProjection;
uniform mat4 matViewT1;
uniform int frame;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_gi_and_ao;

#include "utils/conversion.glsl";
#include "utils/random.glsl";
#include "utils/helpers.glsl";
#include "utils/gl.glsl";

const vec3[] ssao_sample_points = {
  vec3(0.021429, 0.059112, 0.07776),
  vec3(0.042287, -0.020052, 0.092332),
  vec3(0.087243, -0.025947, 0.068744),
  vec3(-0.001496, -0.027043, 0.128824),
  vec3(-0.088486, -0.099508, 0.081747),
  vec3(0.108582, 0.02973, 0.15043),
  vec3(-0.131723, 0.143868, 0.115246),
  vec3(-0.137142, -0.089451, 0.21753),
  vec3(-0.225405, 0.214709, 0.09337),
  vec3(-0.152747, 0.129375, 0.328595),
  vec3(0.155238, 0.146029, 0.398102),
  vec3(-0.205277, 0.358875, 0.3242),
  vec3(0.129059, 0.579216, 0.124064),
  vec3(-0.427557, -0.123908, 0.53261),
  vec3(0.661657, -0.296235, 0.311568),
  vec3(0.175674, 0.013574, 0.87342)
};

vec2 texel_size = 1.0 / screenSize;

vec2 rotatedVogelDisc(int samples, int index) {
  #if USE_DENOISING == 1
    float f = frame % 4;
  #else
    float f = 0;
  #endif

  float rotation = noise(1.0 + f) * 3.141592 * 2.0;
  float theta = 2.4 * index + rotation;
  float radius = sqrt(float(index) + 0.5) / sqrt(float(samples));

  return radius * vec2(cos(theta), sin(theta));
}

float getScreenSpaceAmbientOcclusionContribution(float fragment_depth, vec3 fragment_position, vec3 fragment_normal) {
  const int TOTAL_SAMPLES = 16;
  // @todo make configurable
  const float radius = 8.0;
  vec3 contribution = vec3(0);
  float linearized_fragment_depth = getLinearizedDepth(fragment_depth);
  float occlusion = 0.0;

  #if USE_DENOISING == 1
    float f = frame % 4;
  #else
    float f = 0;
  #endif

  vec3 random_vector = vec3(noise(1.0 + f), noise(2.0 + f), noise(3.0 + f));
  vec3 tangent = normalize(random_vector - fragment_normal * dot(random_vector, fragment_normal));
  vec3 bitangent = cross(fragment_normal, tangent);
  mat3 tbn = mat3(tangent, bitangent, fragment_normal);

  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    vec3 sample_offset = tbn * ssao_sample_points[i];
    vec3 world_sample_position = fragment_position + sample_offset * radius;
    // @hack invert Z
    vec3 view_sample_position = glVec3(matView * glVec4(world_sample_position));
    vec2 screen_sample_position = getScreenCoordinates(view_sample_position, matProjection);
    float sample_depth = textureLod(texColorAndDepth, screen_sample_position, 1).w;
    float linear_sample_depth = getLinearizedDepth(sample_depth);

    if (linear_sample_depth < view_sample_position.z) {
      float occluder_distance = view_sample_position.z - linear_sample_depth;
      float occlusion_factor = mix(1.0, 0.0, saturate(occluder_distance / radius));

      occlusion += occlusion_factor;
    }
  }

  return occlusion / float(TOTAL_SAMPLES);
}

vec3 getScreenSpaceGlobalIlluminationContribution(float fragment_depth, vec3 fragment_position, vec3 fragment_normal) {
  const int TOTAL_SAMPLES = 8;
  const float max_sample_radius = 750.0;
  const float max_brightness = 100.0;
  vec3 global_illumination = vec3(0.0);

  float linearized_fragment_depth = getLinearizedDepth(fragment_depth);
  float radius = max_sample_radius * saturate(1.0 / (linearized_fragment_depth * 0.01));

  // Bounce a ray off the surface and sample points
  // around the bounce ray screen coordinates
  vec3 camera_to_fragment = normalize(fragment_position - cameraPosition);
  vec3 reflection_vector = reflect(camera_to_fragment, fragment_normal);
  vec3 world_bounce_ray = fragment_position + reflection_vector * 10.0;
  // @hack invert Z
  vec3 view_bounce_ray = glVec3(matView * glVec4(world_bounce_ray));
  vec2 bounce_ray_coords = getScreenCoordinates(view_bounce_ray, matProjection);

  for (int i = 0; i < TOTAL_SAMPLES; i++) {
    vec2 offset = texel_size * radius * rotatedVogelDisc(TOTAL_SAMPLES, i);
    vec2 coords = bounce_ray_coords + offset;

    if (isOffScreen(coords, 0.0)) {
      continue;
    }

    vec4 sample_color_and_depth = textureLod(texColorAndDepth, coords, 3);

    // @todo why is this necessary? why are certain sample
    // color components < 0? note: this is to do with
    // probe reflectors
    sample_color_and_depth.r = saturate(sample_color_and_depth.r);
    sample_color_and_depth.g = saturate(sample_color_and_depth.g);
    sample_color_and_depth.b = saturate(sample_color_and_depth.b);

    // Diminish illumination where the sample emits
    // less incident bounce light onto the fragment
    vec3 sample_position = getWorldPosition(sample_color_and_depth.w, fragUv + offset, matInverseProjection, matInverseView);
    float sample_distance = distance(fragment_position, sample_position);
    vec3 normalized_fragment_to_sample = (sample_position - fragment_position) / sample_distance;
    float incidence_factor = max(0.0, dot(fragment_normal, normalized_fragment_to_sample));

    // Diminish illumination with distance
    float distance_factor = max_brightness * saturate(1.0 / sample_distance) * saturate(linearized_fragment_depth / 100.0);

    global_illumination += sample_color_and_depth.rgb * incidence_factor * distance_factor;
  }

  return global_illumination / float(TOTAL_SAMPLES);
}

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec3 fragment_position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
  vec3 fragment_normal = texture(texNormalAndEmissivity, fragUv).xyz;
  vec3 global_illumination = vec3(0.0);
  float ambient_occlusion = 0.0;

  #if USE_SCREEN_SPACE_GLOBAL_ILLUMINATION == 1
    global_illumination = getScreenSpaceGlobalIlluminationContribution(frag_color_and_depth.w, fragment_position, fragment_normal);
  #endif

  #if USE_SCREEN_SPACE_AMBIENT_OCCLUSION == 1
    ambient_occlusion = getScreenSpaceAmbientOcclusionContribution(frag_color_and_depth.w, fragment_position, fragment_normal);
  #endif

  out_gi_and_ao = vec4(global_illumination * 0.75, ambient_occlusion * 0.5);

  #if USE_DENOISING == 1
    vec3 view_fragment_position_t1 = glVec3(matViewT1 * glVec4(fragment_position));
    vec2 frag_uv_t1 = getScreenCoordinates(view_fragment_position_t1.xyz, matProjection);
    vec4 sample_t1 = texture(texIndirectLightT1, frag_uv_t1);
    float linearized_fragment_depth = getLinearizedDepth(frag_color_and_depth.w);
    float weight = 1.0;
    float sample_sum = 0.0;

    float sample_depth = textureLod(texColorAndDepth, frag_uv_t1, 1).w;
    float linear_sample_depth = getLinearizedDepth(sample_depth);

    // if (distance(linearized_fr/agment_depth, linear_sample_depth) < 1.0) {
      out_gi_and_ao += sample_t1 * weight;
      sample_sum += weight;
    // }

    const float spatial_spread_size = 4.0;
    const float spatial_spread_weight = 0.3;

    for (int x = -2; x <= 2; x++) {
      for (int y = -2; y <= 2; y++) {
        vec2 offset = vec2(x, y) * texel_size * spatial_spread_size;
        vec2 sample_uv = frag_uv_t1 + offset;
        float sample_depth = textureLod(texColorAndDepth, sample_uv, 1).w;
        float linear_sample_depth = getLinearizedDepth(sample_depth);

        // if (distance(linearized_fragment_depth, linear_sample_depth) < 2.0) {
          out_gi_and_ao += texture(texIndirectLightT1, sample_uv) * weight * spatial_spread_weight;
          sample_sum += weight * spatial_spread_weight;
        // }
      }
    }

    out_gi_and_ao /= (1.0 + sample_sum);
  #endif
}