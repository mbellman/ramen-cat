#version 460 core

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;
uniform float time;

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

vec2 getPixelCoords() {
  return gl_FragCoord.xy / screenSize;
}

vec2 createRadialWave(vec2 offset) {
  const float TAU = 3.141592 * 2.0;
  // @todo make configurable
  const float speed = 1.0;
  const float frequency = 10.0;

  float u = fragUv.x;
  float v = fragUv.y;
  float radius = length(fragUv - vec2(0.5) + offset);
  float t = -time * speed;
  float r = radius * frequency * TAU;

  return vec2(
    sin(t + r + u),
    cos(t + r + v)
  );
}

vec3 getNormal(vec3 world_position) {
  float wx = world_position.x;
  float wz = world_position.z;
  float t = time;
  vec2 n = vec2(0);

  // @todo make configurable
  n += createRadialWave(vec2(0)) * 0.5;
  n += createRadialWave(vec2(0.3, -0.2)) * 0.4;
  n += createRadialWave(vec2(-0.7, 0.3)) * 0.3;

  n.x += 0.5 * sin(t + wx * 0.2 + sin(t * 3 + wz * 0.2));
  n.y += 0.5 * sin(t + wz * 0.2 + sin(t * 3 + wx * 0.2));

  n.x += 0.3 * sin(wx * 0.3 + sin(wz * 0.3));
  n.y += 0.3 * sin(wz * 0.3 + sin(wx * 0.3));

  n.x += 0.1 * sin(time + wz * 2.0 + sin(time + wx));
  n.y += 0.1 * sin(time + wx * 2.0 + sin(time + wz)); 

  vec3 n_normal = normalize(fragNormal);
  vec3 n_tangent = normalize(fragTangent);
  vec3 n_bitangent = normalize(fragBitangent);

  mat3 tbn_matrix = mat3(
    n_tangent,
    n_bitangent,
    n_normal
  );

  vec3 tangent_normal = vec3(n.x, n.y, 1.0);
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
  vec3 water_color = refracted_color_and_depth.rgb;

  if (refracted_color_and_depth.w < gl_FragCoord.z) {
    water_color = vec3(0);
  }

  water_color *= fresnel_factor;

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
    float depth_distance = getLinearizedDepth(reflection_color_and_depth.w) - view_reflection_ray.z;
    float alpha = max(0.0, min(1.0, depth_distance / 20.0));

    reflection_color = mix(reflection_color_and_depth.rgb, sky_color, alpha);
  }

  water_color += reflection_color * (1.0 - fresnel_factor);
  water_color *= fragColor;

  out_color_and_depth = vec4(water_color, gl_FragCoord.z);
}