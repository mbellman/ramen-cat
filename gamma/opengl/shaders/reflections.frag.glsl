#version 460 core

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform vec3 cameraPosition;
uniform mat4 matView;
uniform mat4 matInverseView;
uniform mat4 matProjection;
uniform mat4 matInverseProjection;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

struct Reflection {
  vec3 color;
  vec2 uv;
  float screen_edge_visibility;
};

// @todo pass these in as uniforms
const float z_near = 1.0;
const float z_far = 10000.0;
const float reflection_factor = 0.5;
const float thickness_threshold = 5.0;

#include "utils/gl.glsl";
#include "utils/conversion.glsl";
#include "utils/random.glsl";
#include "utils/skybox.glsl";
#include "utils/helpers.glsl";

/**
 * Returns a reflected light intensity factor, reduced
 * by proximity of the sample to the screen edges.
 */
float getReflectionIntensity(vec2 uv) {
  const float X_TAPER = 0.05;
  const float Y_TAPER = 0.2;

  float intensity = 1.0;

  if (uv.x < X_TAPER) intensity *= (uv.x / X_TAPER);
  if (uv.x > (1.0 - X_TAPER)) intensity *= 1.0 - (uv.x - (1.0 - X_TAPER)) * (1.0 / X_TAPER);
  if (uv.y > (1.0 - Y_TAPER)) intensity *= 1.0 - (uv.y - (1.0 - Y_TAPER)) * (1.0 / Y_TAPER);

  return clamp(intensity, 0, 1);
}

/**
 * Returns a Reflection by converging on the likely point
 * that a ray intersects geometry, determined in screen space.
 * We march the ray forward or backward, depending on whether
 * an intersection does not or does occur, respectively, using
 * decreasing steps each time in a binary search.
 */
Reflection getRefinedReflection(
  vec3 view_reflecting_surface_position,
  vec3 normalized_view_reflection_ray,
  vec3 view_starting_ray,
  float march_step_size
) {
  const int TOTAL_REFINEMENT_STEPS = 6;

  vec3 ray = view_starting_ray;
  vec3 ray_step = normalized_view_reflection_ray * march_step_size;
  vec3 refined_color = vec3(0);
  vec2 refined_uv = vec2(0);
  float test_depth;

  for (int i = 0; i < TOTAL_REFINEMENT_STEPS; i++) {
    // Decrease step size by half and advance the ray
    ray_step *= 0.5;
    ray += ray_step;
    refined_uv = getScreenCoordinates(ray, matProjection);

    vec4 test = texture(texColorAndDepth, refined_uv);

    test_depth = getLinearizedDepth(test.w);
    refined_color = test.rgb;

    // If the ray is still intersecting the geometry,
    // advance a full step back. On the next cycle
    // we'll halve the step size and advance forward
    // again, converging on our likely reflection point.
    if (test_depth < ray.z && test_depth > view_reflecting_surface_position.z) {
      ray -= ray_step;
    }
  }

  // Disable reflections of points at the far plane
  float intensity = test_depth >= z_far ? 0.0 : getReflectionIntensity(refined_uv);

  return Reflection(refined_color, refined_uv, intensity);
}

/**
 * Returns a Reflection at a given reflecting surface fragment,
 * which may or may not reflect actual screen-space geometry.
 * Surfaces with reflection rays which 'miss' geometry instead
 * reflect the sky.
 */
Reflection getReflection(
  vec3 view_reflecting_surface_position,
  vec3 normalized_view_reflection_ray,
  float march_step_size
) {
  const int TOTAL_MARCH_STEPS = 24;

  vec3 ray = view_reflecting_surface_position;
  vec3 previous_ray = ray;
  float current_step_size = march_step_size;

  for (int i = 0; i < TOTAL_MARCH_STEPS; i++) {
    vec3 ray_step = normalized_view_reflection_ray * current_step_size;

    ray += ray_step;

    vec2 uv = getScreenCoordinates(ray, matProjection);

    if (isOffScreen(uv, 0.0) || ray.z >= z_far) {
      // Bail if the ray goes offscreen or beyond the far plane
      break;
    }

    vec4 test = texture(texColorAndDepth, uv);
    float test_depth = getLinearizedDepth(test.w);
    float ray_to_geometry_distance = abs(ray.z - test_depth);

    // A reflection occurs when:
    if (
      // 1) The ray is behind the geometry
      ray.z > test_depth &&
      (normalized_view_reflection_ray.z > 0
        // 2) For outgoing rays, the previous ray
        // was 'in front' of the geometry, within
        // a thickness threshold
        ? (test_depth - previous_ray.z > -thickness_threshold)
        // 3) For incoming rays, the previous ray
        // was behind the geometry, within a thickness
        // threshold
        : (previous_ray.z - test_depth < thickness_threshold)
      )
    ) {
      return getRefinedReflection(view_reflecting_surface_position, normalized_view_reflection_ray, ray - ray_step, current_step_size);
    }

    current_step_size *= 1.3;

    previous_ray = ray;
  }

  return Reflection(vec3(0), vec2(0), 0);
}

float getInitialMarchStepSize() {
  const float step_size_kernel[] = {
    1.0, 0.92, 1.03, 0.95,
    0.93, 0.98, 1.08, 1.02,
    1.1, 1.03, 0.92, 0.9,
    0.97, 0.94, 1.5, 0.99
  };

  int x = int(gl_FragCoord.x) % 4;
  int y = int(gl_FragCoord.y) % 4;
  int idx = y * 4 + x;

  return step_size_kernel[idx];
}

void main() {
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec4 frag_normal_and_emissivity = texture(texNormalAndEmissivity, fragUv);
  vec3 frag_world_position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
  vec3 camera_to_fragment = frag_world_position - cameraPosition;
  vec3 normalized_camera_to_fragment = normalize(camera_to_fragment);
  vec3 frag_world_normal = frag_normal_and_emissivity.rgb;

  // @hack invert Z
  vec4 frag_view_position = glVec4(matView * glVec4(frag_world_position));
  vec4 frag_view_normal = glVec4(transpose(matInverseView) * glVec4(frag_world_normal));
  vec3 world_reflection_vector = reflect(normalized_camera_to_fragment, frag_world_normal);
  vec3 normalized_view_reflection_ray = reflect(normalize(frag_view_position.xyz), frag_view_normal.xyz);
  float march_step_size = getInitialMarchStepSize();

  Reflection reflection = getReflection(frag_view_position.xyz, normalized_view_reflection_ray, march_step_size);
  vec3 baseColor = frag_color_and_depth.rgb * (1.0 - reflection_factor);
  vec3 reflectionColor = reflection.color * reflection.screen_edge_visibility * reflection_factor;
  vec3 skyColor = getSkyColor(world_reflection_vector) * reflection_factor * (1.0 - reflection.screen_edge_visibility);

  out_color_and_depth = vec4(baseColor + reflectionColor + skyColor, frag_color_and_depth.w);
}