#version 460 core

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;
uniform float time;

uniform float zNear;
uniform float zFar;

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/gl.glsl";
#include "utils/conversion.glsl";
#include "utils/skybox.glsl";
#include "utils/helpers.glsl";
#include "utils/random.glsl";

vec2 getPixelCoords() {
  return gl_FragCoord.xy / screenSize;
}

vec2 createRadialWave(vec3 world_position, vec2 offset) {
  const float TAU = 3.141592 * 2.0;

  // @todo make configurable
  const float speed = 0.3;
  const float scale = 1000.0;

  float radius = length(world_position.xz / scale - offset);
  float t = time * speed;
  float r = radius * TAU;

  return vec2(
    sin(t + r),
    cos(t + r)
  );
}

vec2 createDirectionalWave(float wx, float wz, vec2 direction, float frequency, float wavelength, float intensity) {
  float w_input = wx * direction.x + wz * direction.y;

  float x = sin(-time * frequency + w_input * wavelength) * intensity;
  float z = cos(-time * frequency + w_input * wavelength) * intensity;

  return intensity * vec2(x, z);
}

vec3 getNormal(vec3 world_position) {
  float wx = world_position.x;
  float wz = world_position.z;
  float t = time;
  vec2 n = vec2(0);

  // @todo make configurable
  n += createDirectionalWave(wx, wz, vec2(0.5, 1), 1, 0.01, 1);
  n += createDirectionalWave(wx, wz, vec2(0.1, 1), 0.6, 0.012, 0.4);
  n += createDirectionalWave(wx, wz, vec2(1, 0.3), 0.5, 0.006, 0.6);
  n += createDirectionalWave(wx, wz, vec2(0.5, 1), 0.3, 0.016, 0.7);
  n += createDirectionalWave(wx, wz, vec2(0, -0.3), 0.6, 0.01, 0.7);
  n += createDirectionalWave(wx, wz, vec2(0.2, 1), 1, 0.03, 0.5);
  n += createDirectionalWave(wx, wz, vec2(1, 0.6), 0.7, 0.03, 0.2);

  n += vec2(simplex_noise(vec2(t * 0.1 + wx * 0.001, t * 0.1 + wz * 0.001))) * 0.6;
  n += vec2(simplex_noise(vec2(t * 0.1 - wx * 0.003, t * 0.1 + wz * 0.003))) * 0.4;
  n += vec2(simplex_noise(vec2(t * 0.1 + wx * 0.007, t * 0.1 - wz * 0.007))) * 0.3;

  vec3 n_normal = normalize(fragNormal);
  vec3 n_tangent = normalize(fragTangent);
  vec3 n_bitangent = normalize(fragBitangent);

  mat3 tbn_matrix = mat3(
    n_tangent,
    n_bitangent,
    n_normal
  );

  // @todo make configurable
  float flatness = 1.0;

  vec3 tangent_normal = vec3(n.x, n.y, flatness);
  vec3 world_normal = normalize(tbn_matrix * tangent_normal);

  world_normal.y += n_normal.y * 3.0;

  return normalize(world_normal);
}

void main() {
  const float REFRACTION_INTENSITY = 1.0;

  vec3 world_position = getWorldPosition(gl_FragCoord.z, getPixelCoords(), matInverseProjection, matInverseView);
  vec3 normalized_fragment_to_camera = normalize(cameraPosition - world_position);
  vec3 color = vec3(1.0);
  vec3 normal = getNormal(world_position);

  // Fresnel effect
  float fresnel_factor = dot(normalized_fragment_to_camera, normal);

  // Hack for when the camera is positioned below/inside the water,
  // causing the fragment-to-camera vector to be pointed away from
  // the surface normal, resulting in a negative dot product and
  // inverting the refracted image color
  if (fresnel_factor < 0) fresnel_factor *= -1;

  // Refraction
  vec3 refraction_ray = refract(normalized_fragment_to_camera, normal, 0.7);
  vec3 world_refraction_ray = world_position + refraction_ray * REFRACTION_INTENSITY;
  // @hack invert Z
  vec3 view_refraction_ray = glVec3(matView * glVec4(world_refraction_ray));
  vec2 refracted_color_coords = getScreenCoordinates(view_refraction_ray, matProjection);
  float sample_depth = texture(texColorAndDepth, getPixelCoords()).w;

  if (sample_depth < 1.0 && isOffScreen(refracted_color_coords, 0.0)) {
    // If the refraction sample coordinates are off-screen,
    // disable refractive effects
    // @todo improve this
    refracted_color_coords = getPixelCoords();
  }

  if (gl_FragCoord.z > sample_depth) {
    // Accommodation for alpha-blended particles, which write to the
    // depth channel of the color and depth texture, but not to the
    // depth buffer, in order to properly blend against themselves.
    // Perform a 'manual' depth test to ensure that particles in front
    // of the refractive geometry aren't overwritten and incorrectly
    // rendered 'behind' it.
    discard;
  }

  vec4 refracted_color_and_depth = texture(texColorAndDepth, refracted_color_coords);
  vec3 water_color = vec3(0);
  
  water_color += refracted_color_and_depth.rgb * fresnel_factor;

  if (refracted_color_and_depth.w < gl_FragCoord.z) {
    water_color = vec3(0);
  }

  // Reflection
  vec3 reflection_ray = reflect(normalized_fragment_to_camera * -1, normal);
  vec3 view_reflection_ray = glVec3(matView * glVec4(world_position + reflection_ray * 5.0));
  vec2 reflected_color_coords = getScreenCoordinates(view_reflection_ray, matProjection);
  vec4 reflection_color_and_depth = texture(texColorAndDepth, reflected_color_coords);
  vec3 sky_color = getSkyColor(reflection_ray);
  vec3 reflection_color = vec3(0);

  if (isOffScreen(reflected_color_coords, 0)) {
    reflection_color = sky_color;
  } else {
    // Fade from geometry reflections to sky reflections depending on
    // how deep into the geometry the reflection ray went. We want a
    // gradual transition from geometry to sky reflections instead of
    // an abrupt cutoff/fallback if no geometry is reflected.
    float depth_distance = getLinearizedDepth(reflection_color_and_depth.w, zNear, zFar) - view_reflection_ray.z;
    float alpha = max(0.0, min(1.0, depth_distance / 20.0));

    reflection_color = mix(reflection_color_and_depth.rgb, sky_color, alpha);
  }

  water_color += reflection_color * (1.0 - fresnel_factor);
  water_color *= fragColor;

  // @todo make water color configurable
  water_color += vec3(0, 0.25, 0.25) * fresnel_factor;

  out_color_and_depth = vec4(water_color, gl_FragCoord.z);
}