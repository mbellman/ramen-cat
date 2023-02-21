#version 460 core

#define USE_DEPTH_OF_FIELD 0

uniform sampler2D texColorAndDepth;

uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;

uniform float screenWarpTime;
uniform vec3 atmosphereColor;

uniform float zNear;
uniform float zFar;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

#include "utils/random.glsl";
#include "utils/conversion.glsl";
#include "utils/helpers.glsl";

vec3 getDepthOfFieldColor(vec3 current_out_color, vec2 uv, float linear_frag_depth) {
  const int MIP_LEVEL = 1;
  const float MAX_DEPTH = 3000.0;

  vec2 texel_size = 1.0 / textureSize(texColorAndDepth, MIP_LEVEL);
  vec3 depth_of_field_color = vec3(0.0);

  vec2 uv1 = uv + vec2(-1.0, 0.0) * texel_size;
  vec2 uv2 = uv + vec2(1.0, 0.0) * texel_size;
  vec2 uv3 = uv + vec2(0.0, -1.0) * texel_size;
  vec2 uv4 = uv + vec2(0.0, 1.0) * texel_size;

  depth_of_field_color += textureLod(texColorAndDepth, uv, MIP_LEVEL).rgb;
  depth_of_field_color += textureLod(texColorAndDepth, uv1, MIP_LEVEL).rgb;
  depth_of_field_color += textureLod(texColorAndDepth, uv2, MIP_LEVEL).rgb;
  depth_of_field_color += textureLod(texColorAndDepth, uv3, MIP_LEVEL).rgb;
  depth_of_field_color += textureLod(texColorAndDepth, uv4, MIP_LEVEL).rgb;
  depth_of_field_color /= 5.0;

  float depth_factor = linear_frag_depth / MAX_DEPTH;
  if (depth_factor > 1.0) depth_factor = 1.0;

  depth_factor *= depth_factor;

  return mix(current_out_color, depth_of_field_color, depth_factor);
}

vec3 getAtmosphericsColor(vec3 current_out_color, vec2 uv, float frag_depth, float linear_frag_depth) {
  vec3 sky_position = getWorldPosition(1.0, uv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 sky_direction = normalize(sky_position);

  // @todo make configurable
  const float horizon_altitude = -2000.0;
  float altitude_above_horizon = cameraPosition.y - horizon_altitude;
  vec2 horizon_direction_2d = normalize(vec2(zFar, -altitude_above_horizon));
  vec2 sky_direction_2d = normalize(vec2(length(sky_direction.xz), sky_direction.y));

  float depth_divisor = frag_depth == 1.0 ? zFar : zFar * 0.85;
  float atmosphere_factor = linear_frag_depth / depth_divisor;

  atmosphere_factor *= sky_direction_2d.y < horizon_direction_2d.y ? 1.0 : pow(dot(sky_direction_2d, horizon_direction_2d), 100);
  atmosphere_factor = atmosphere_factor > 1 ? 1 : atmosphere_factor;
  atmosphere_factor = isnan(atmosphere_factor) ? 0 : atmosphere_factor;

  return mix(current_out_color, atmosphereColor, atmosphere_factor);
}

vec3 getToonShadedColor(vec3 current_out_color, vec2 uv, float linear_frag_depth) {
  // @todo make configurable
  const float OUTLINE_THICKNESS = 1.5;

  vec2 texel_size = 1.0 / textureSize(texColorAndDepth, 0);
  vec2 uv1 = uv + texel_size * vec2(-1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv2 = uv + texel_size * vec2(1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv3 = uv + texel_size * vec2(0, -1.0) * OUTLINE_THICKNESS;
  vec2 uv4 = uv + texel_size * vec2(0, 1.0) * OUTLINE_THICKNESS;
  float depth1 = getLinearizedDepth(texture(texColorAndDepth, uv1).w, zNear, zFar);
  float depth2 = getLinearizedDepth(texture(texColorAndDepth, uv2).w, zNear, zFar);
  float depth3 = getLinearizedDepth(texture(texColorAndDepth, uv3).w, zNear, zFar);
  float depth4 = getLinearizedDepth(texture(texColorAndDepth, uv4).w, zNear, zFar);
  float threshold = 1000.0 * (linear_frag_depth / zFar);

  // @todo @bug fix buggy 'outline' darkening on flat surfaces at grazing angles.
  // We'll likely have to check normals as well.
  if (
    depth1 - linear_frag_depth > threshold ||
    depth2 - linear_frag_depth > threshold ||
    depth3 - linear_frag_depth > threshold ||
    depth4 - linear_frag_depth > threshold
  ) {
    float alpha = saturate((linear_frag_depth + zFar * 0.5) / zFar);

    current_out_color = mix(vec3(0), current_out_color, alpha);
  }

  return current_out_color;
}

void main() {
  // Apply screen warp
  float aspect_ratio = 1920.0 / 1080.0;
  vec2 frag_uv_ratio = vec2(1.0, 1 / aspect_ratio);
  float screen_warp_radius = length(fragUv * frag_uv_ratio - vec2(0.5, 0.5) * frag_uv_ratio);
  float screen_warp_offset = min(1.0, distance(screen_warp_radius, screenWarpTime));
  float screen_warp_displacement = pow(1.0 - screen_warp_offset, 30);
  vec2 screen_warp_uv = (fragUv - 0.5) * (1.0 - screen_warp_displacement * 0.1) + 0.5;
  vec4 frag_color_and_depth = texture(texColorAndDepth, screen_warp_uv);
  float linear_frag_depth = getLinearizedDepth(frag_color_and_depth.w, zNear, zFar);

  out_color = frag_color_and_depth.rgb;

  #if USE_DEPTH_OF_FIELD == 1
    out_color = getDepthOfFieldColor(out_color, screen_warp_uv, linear_frag_depth);
  #endif

  // @todo make atmospherics optional via a flag
  out_color = getAtmosphericsColor(out_color, screen_warp_uv, frag_color_and_depth.w, linear_frag_depth);
  out_color = getToonShadedColor(out_color, screen_warp_uv, linear_frag_depth);

  // @todo gamma correction/tone-mapping
  // out_color = pow(out_color, vec3(1 / 2.2));
}