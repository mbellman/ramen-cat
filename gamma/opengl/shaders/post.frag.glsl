#version 460 core

#define USE_DEPTH_OF_FIELD 0
#define USE_HORIZON_ATMOSPHERE 1

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndMaterial;

uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;
uniform vec3 playerPosition;

uniform float screenWarpTime;
uniform vec3 atmosphereColor;

uniform float zNear;
uniform float zFar;

// Game-specific modifications
uniform vec3 redshiftSpawn;
uniform float redshiftInProgress;
uniform float redshiftOutProgress;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

#include "utils/random.glsl";
#include "utils/conversion.glsl";
#include "utils/helpers.glsl";

vec3 getAtmosphericsColor(vec3 current_out_color, vec2 uv, float frag_depth, float linear_frag_depth, vec3 world_position) {
  // @todo make configurable
  const float atmosphere_density = 1.0;
  const float atmosphere_distance_limit = 0.8;
  const float max_atmosphere_altitude = 20000.0;
  const float horizon_altitude = -2000.0;

  vec3 sky_position = getWorldPosition(1.0, uv, matInverseProjection, matInverseView) - cameraPosition;
  vec3 sky_direction = normalize(sky_position);

  float altitude_above_horizon = cameraPosition.y - horizon_altitude;
  vec2 horizon_direction_2d = normalize(vec2(zFar, -altitude_above_horizon));
  vec2 sky_direction_2d = normalize(vec2(length(sky_direction.xz), sky_direction.y));

  float depth_divisor = (frag_depth == 1.0 ? zFar : zFar * 0.85) * atmosphere_distance_limit;
  float frag_distance = frag_depth == 1.0 ? linear_frag_depth : distance(world_position, cameraPosition);
  float frag_depth_ratio = min(1.0, frag_distance / depth_divisor);
  float atmosphere_altitude_thickness = max(frag_depth_ratio, saturate(1 - world_position.y / max_atmosphere_altitude));
  float atmosphere_intensity = pow(frag_depth_ratio, 1.0 / atmosphere_density);
  float atmosphere_factor = atmosphere_altitude_thickness * atmosphere_intensity;

  atmosphere_factor *= sky_direction_2d.y < horizon_direction_2d.y ? 1.0 : pow(dot(sky_direction_2d, horizon_direction_2d), 20);
  atmosphere_factor = atmosphere_factor > 1 ? 1 : atmosphere_factor;
  atmosphere_factor = isnan(atmosphere_factor) ? 0 : atmosphere_factor;

  return mix(current_out_color, atmosphereColor, atmosphere_factor);
}

vec3 getToonShadedColor(vec3 current_out_color, vec2 uv, float depth, float linear_frag_depth) {
  const float OUTLINE_THICKNESS = 1.0;

  // Get the depth values for the top/left/right/bottom pixels
  vec2 texel_size = 1.0 / textureSize(texColorAndDepth, 0);
  vec2 uv1 = uv + texel_size * vec2(-1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv2 = uv + texel_size * vec2(1.0, 0) * OUTLINE_THICKNESS;
  vec2 uv3 = uv + texel_size * vec2(0, -1.0) * OUTLINE_THICKNESS;
  vec2 uv4 = uv + texel_size * vec2(0, 1.0) * OUTLINE_THICKNESS;

  vec4 t1 = texture(texColorAndDepth, uv1);
  vec4 t2 = texture(texColorAndDepth, uv2);
  vec4 t3 = texture(texColorAndDepth, uv3);
  vec4 t4 = texture(texColorAndDepth, uv4);

  float depth1 = getLinearizedDepth(t1.w, zNear, zFar);
  float depth2 = getLinearizedDepth(t2.w, zNear, zFar);
  float depth3 = getLinearizedDepth(t3.w, zNear, zFar);
  float depth4 = getLinearizedDepth(t4.w, zNear, zFar);

  // Determine how sharp of an angle the surface is being viewed at
  vec3 world_position = getWorldPosition(depth, uv, matInverseProjection, matInverseView);
  vec3 normalized_frag_to_camera = normalize(cameraPosition - world_position);
  vec3 fragment_normal = texture(texNormalAndMaterial, uv).xyz;
  float grazing_factor = saturate(pow(1.0 - dot(normalized_frag_to_camera, fragment_normal), 10));

  // Calculate a distance threshold representing how far away a neighboring
  // pixel must be to consider this a silhouette edge
  float depth_ratio = linear_frag_depth / zFar;
  float threshold_factor = 1000.0 + (zFar / 2.0) * grazing_factor;
  float distance_threshold = threshold_factor * depth_ratio;

  if (
    depth1 - linear_frag_depth > distance_threshold ||
    depth2 - linear_frag_depth > distance_threshold ||
    depth3 - linear_frag_depth > distance_threshold ||
    depth4 - linear_frag_depth > distance_threshold
  ) {
    float alpha = saturate(0.5 + linear_frag_depth / zFar);

    current_out_color = mix(vec3(0), current_out_color, alpha);
  } else if (depth_ratio < 1.0) {
    float fade_factor = 0.05 + pow(saturate(linear_frag_depth / 20000.0), 3) * 0.3;

    current_out_color = mix(current_out_color, atmosphereColor, fade_factor);
    current_out_color = mix(current_out_color, pow(current_out_color, vec3(1 / 0.5)) * 2.0, fade_factor);
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
  vec3 world_position = getWorldPosition(frag_color_and_depth.w, screen_warp_uv, matInverseProjection, matInverseView);

  out_color = frag_color_and_depth.rgb;

  out_color = getToonShadedColor(out_color, screen_warp_uv, frag_color_and_depth.w, linear_frag_depth);

  // @todo make atmospherics optional via a flag
  #if USE_HORIZON_ATMOSPHERE == 1
    out_color = getAtmosphericsColor(out_color, screen_warp_uv, frag_color_and_depth.w, linear_frag_depth, world_position);
  #endif

  // Game-specific modifications below
  // ---------------------------------

  // Redshifting/Torii Gate zones
  {
    vec3 normalized_frag_to_camera = normalize(cameraPosition - world_position);
    vec3 fragment_normal = normalize(texture(texNormalAndMaterial, screen_warp_uv).xyz);
    float in_progress = min(1.0, redshiftInProgress);
    float out_progress = min(1.0, redshiftOutProgress);
    float redshift_in_radius = zFar * (redshiftInProgress < 1.0 ? pow(redshiftInProgress, 3) : redshiftInProgress);
    float redshift_out_radius = zFar * (redshiftOutProgress < 1.0 ? pow(redshiftOutProgress, 3) : redshiftOutProgress);
    vec3 base_out_color = out_color;
    float distance_from_redshift_spawn = distance(redshiftSpawn, world_position);

    if (distance_from_redshift_spawn < redshift_in_radius) {
      float nDotC = dot(normalized_frag_to_camera, fragment_normal);
      float frag_distance = frag_color_and_depth.w == 1.0 ? linear_frag_depth : distance(world_position, cameraPosition);
      float view_angle_redshift = frag_color_and_depth.w == 1.0 ? 1.0 : pow(1.0 - max(0.0, nDotC), 4);
      float distance_redshift = frag_color_and_depth.w == 1.0 ? 1.0 : saturate(1.5 * (frag_distance / zFar));
      float redshift = 1.5 * max(view_angle_redshift, distance_redshift);
      float alpha = 1.0 - pow(distance_from_redshift_spawn / redshift_in_radius, 3);
      vec3 redshifted_out_color = out_color;

      redshifted_out_color *= vec3(0.3, 0.4, 0.5);
      redshifted_out_color += vec3(redshift, 0, 0);

      out_color = mix(out_color, redshifted_out_color, alpha);
    }

    if (distance_from_redshift_spawn < redshift_out_radius) {
      float alpha = 1.0 - pow(distance_from_redshift_spawn / redshift_out_radius, 3);

      out_color = mix(out_color, base_out_color, alpha);
    }

    float torii_gate_zone_factor = min(1.0, in_progress) - min(1.0, out_progress);
    float vignette_factor = max(0.0, distance(screen_warp_uv, vec2(0.5)) * torii_gate_zone_factor);

    out_color = mix(out_color, vec3(0.2, 0, 0), vignette_factor);
  }

  // @todo gamma correction/tone-mapping
  // out_color = pow(out_color, vec3(1 / 2.2));
}