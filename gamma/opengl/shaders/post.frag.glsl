#version 460 core

#define USE_DEPTH_OF_FIELD 0

uniform sampler2D texColorAndDepth;

uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;
uniform float time;

uniform float zNear;
uniform float zFar;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

#include "utils/random.glsl";
#include "utils/conversion.glsl";

/**
 * Applies a noise filter to colors to reduce banding
 * artifacts, strengthening the effect in darker ranges
 * where banding is more apparent.
 */
vec3 deband(vec3 color) {
  float brightness = max(color.r, 0) + max(color.g, 0) + max(color.b, 0);
  float divisor = brightness * 150.0;

  if (divisor == 0.0) {
    return vec3(0);
  }

  return color * (1.0 + random(0.0, 1.0) / divisor);
}

void main() {
  // @todo cleanup
  float aspect_ratio = 1920.0 / 1080.0;
  vec2 frag_uv_ratio = vec2(1.0, 1 / aspect_ratio);
  float radius = length(fragUv * frag_uv_ratio - vec2(0.5, 0.5) * frag_uv_ratio);
  float offset = min(1.0, distance(radius, time));
  float displacement = pow(1.0 - offset, 30);
  vec2 adjusted_uv = (fragUv - 0.5) * (1.0 - displacement * 0.1) + 0.5;
  vec4 frag_color_and_depth = texture(texColorAndDepth, adjusted_uv);

  #if USE_DEPTH_OF_FIELD == 1
    const int MIP_LEVEL = 1;
    const float MAX_DEPTH = 3000.0;

    vec2 texel_size = 1.0 / textureSize(texColorAndDepth, MIP_LEVEL);
    vec3 depth_of_field_color = vec3(0.0);

    vec2 uv1 = adjusted_uv + vec2(-1.0, 0.0) * texel_size;
    vec2 uv2 = adjusted_uv + vec2(1.0, 0.0) * texel_size;
    vec2 uv3 = adjusted_uv + vec2(0.0, -1.0) * texel_size;
    vec2 uv4 = adjusted_uv + vec2(0.0, 1.0) * texel_size;

    depth_of_field_color += textureLod(texColorAndDepth, adjusted_uv, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv1, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv2, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv3, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv4, MIP_LEVEL).rgb;
    depth_of_field_color /= 5.0;

    float depth_factor = getLinearizedDepth(frag_color_and_depth.w, zNear, zFar) / MAX_DEPTH;

    if (depth_factor > 1.0) depth_factor = 1.0;

    depth_factor *= depth_factor;

    out_color = mix(frag_color_and_depth.rgb, depth_of_field_color, depth_factor);
  #else
    out_color = frag_color_and_depth.rgb;
  #endif

  // @todo make atmospherics optional via a flag
  // @bug This produces an outline on geometry near the horizon line due to
  // accumulation buffer texture filtering. If we disable that, the general
  // visual quality is notably worse until we add FXAA or similar. We can
  // also do an atmosphere pass in the skybox shader, and then add a remaining
  // depth/height-based atmospheric scattering effect here.
  vec3 sky_position = getWorldPosition(1.0, adjusted_uv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 sky_direction = normalize(sky_position);

  // @todo make configurable
  const float horizon_altitude = -2000.0;
  float altitude_above_horizon = cameraPosition.y - horizon_altitude;
  vec2 horizon_direction_2d = normalize(vec2(zFar, -altitude_above_horizon));
  vec2 sky_direction_2d = normalize(vec2(length(sky_direction.xz), sky_direction.y));

  // @todo make configurable
  vec3 atmosphere_color = vec3(1.0, 0.8, 1.0);
  float depth_divisor = frag_color_and_depth.w == 1.0 ? zFar : zFar * 0.9;
  float atmosphere_factor = getLinearizedDepth(frag_color_and_depth.w, zNear, zFar) / depth_divisor;

  atmosphere_factor *= pow(dot(sky_direction_2d, horizon_direction_2d), 100);
  atmosphere_factor = atmosphere_factor > 1 ? 1 : atmosphere_factor;
  atmosphere_factor = isnan(atmosphere_factor) ? 0 : atmosphere_factor;

  out_color = mix(out_color, atmosphere_color, atmosphere_factor);

  // @todo gamma correction/tone-mapping
  // out_color = pow(out_color, vec3(1 / 2.2)) - 0.2;
}