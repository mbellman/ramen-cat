#include "utils/gl.glsl";

/**
 * Reconstructs a fragment's world position from depth,
 * using the inverse projection/view matrices to transform
 * the fragment coordinates back into world space.
 *
 * @todo can we replace frag_uv with gl_FragCoord.xy?
 */
vec3 getWorldPosition(float depth, vec2 frag_uv, mat4 inverse_projection, mat4 inverse_view) {
  float z = depth * 2.0 - 1.0;
  vec4 clip = vec4(frag_uv * 2.0 - 1.0, z, 1.0);
  vec4 view_position = inverse_projection * clip;

  view_position /= view_position.w;

  vec4 world_position = inverse_view * view_position;

  return glVec3(world_position.xyz);
}

/**
 * Maps a nonlinear [0, 1] depth value to a linearized
 * depth between the near and far planes.
 */
float getLinearizedDepth(float depth) {
  float clip_depth = 2.0 * depth - 1.0;
  // @todo import from a 'utils/constants.glsl' file; use uniforms
  float near_plane = 1.0;
  float far_plane = 10000.0;

  return 2.0 * near_plane * far_plane / (far_plane + near_plane - clip_depth * (far_plane - near_plane));
}

/**
 * Maps a linear [near, far] depth value to a nonlinear
 * [0, 1] depth value.
 */
float getFragDepth(float linearized_depth, mat4 projection) {
  float a = projection[2][2];
  float b = projection[3][2];

  return 0.5 * (-a * linearized_depth + b) / linearized_depth + 0.5;
}

/**
 * Returns the 2D screen coordinates, normalized to the range
 * [0.0, 1.0], corresponding to a point in view space.
 */
vec2 getScreenCoordinates(vec3 view_position, mat4 projection) {
  // @hack invert Z
  vec4 proj = projection * glVec4(view_position);
  vec3 clip = proj.xyz / proj.w;

  return clip.xy * 0.5 + 0.5;
}