#version 460 core

#define BLOCK_SKYLIGHT_IN_SHADOW 1

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform sampler2D texClouds;
uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;

uniform sampler2D texShadowMap;
uniform mat4 matLightViewProjection;

uniform float time;
uniform float zNear;
uniform float zFar;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 atmosphereColor;
uniform float altitude;
uniform float turbulence;

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
  n += createDirectionalWave(wx, wz, vec2(0.5, 1), 1, 0.005, 0.8);
  n += createDirectionalWave(wx, wz, vec2(0.1, 1), 0.6, 0.012, 0.4);
  n += createDirectionalWave(wx, wz, vec2(1, 0.3), 0.5, 0.006, 0.6);
  n += createDirectionalWave(wx, wz, vec2(0.5, 1), 0.3, 0.016, 0.7);
  n += createDirectionalWave(wx, wz, vec2(0, -0.3), 0.6, 0.01, 0.7);
  n += createDirectionalWave(wx, wz, vec2(0.2, 1), 1, 0.03, 0.5);
  n += createDirectionalWave(wx, wz, vec2(1, 0.6), 0.7, 0.03, 0.2);

  n += vec2(simplex_noise(vec2(t * 0.08 + wx * 0.0002, t * 0.1 + wz * 0.00005)));
  n += vec2(simplex_noise(vec2(t * 0.1 - wx * 0.0005, t * 0.1 - wz * 0.0005))) * 0.3;
  n += vec2(simplex_noise(vec2(t * 0.1 + wx * 0.002, t * 0.1 - wz * 0.002))) * 0.2;

  n *= turbulence;

  vec3 n_normal = normalize(fragNormal);
  vec3 n_tangent = normalize(fragTangent);
  vec3 n_bitangent = normalize(fragBitangent);

  mat3 tbn_matrix = mat3(
    n_tangent,
    n_bitangent,
    n_normal
  );

  vec3 tangent_normal = vec3(n.x, n.y, 1.0);

  return normalize(tbn_matrix * tangent_normal);
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
  const float REFRACTION_INTENSITY = 20.0;

  vec3 world_position = getWorldPosition(gl_FragCoord.z, getPixelCoords(), matInverseProjection, matInverseView);
  vec3 normalized_fragment_to_camera = normalize(cameraPosition - world_position);
  vec3 normal = getNormal(world_position);

  // Fresnel effect
  float fresnel_factor = dot(normalized_fragment_to_camera, normal);

  // Hack for when the camera is positioned below/inside the ocean,
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
  vec4 sample_color_and_depth = texture(texColorAndDepth, getPixelCoords());

  if (sample_color_and_depth.w < 1.0 && isOffScreen(refracted_color_coords, 0.0)) {
    // If the refraction sample coordinates are off-screen,
    // disable refractive effects
    // @todo improve this
    refracted_color_coords = getPixelCoords();
  }

  if (gl_FragCoord.z > sample_color_and_depth.w) {
    // Accommodation for alpha-blended particles, which write to the
    // depth channel of the color and depth texture, but not to the
    // depth buffer, in order to properly blend against themselves.
    // Perform a 'manual' depth test to ensure that particles in front
    // of the refractive geometry aren't overwritten and incorrectly
    // rendered 'behind' it.
    discard;
  }

  vec4 refracted_color_and_depth = texture(texColorAndDepth, refracted_color_coords);
  vec3 refraction_color = refracted_color_and_depth.rgb;
  vec3 ocean_color = vec3(0);
  
  if (refracted_color_and_depth.w < gl_FragCoord.z) {
    // Don't refract geometry in front of the ocean surface
    refraction_color = sample_color_and_depth.rgb;
  }

  // Reflection
  vec3 reflection_ray = reflect(normalized_fragment_to_camera * -1, normal);
  vec3 view_reflection_ray = glVec3(matView * glVec4(world_position + reflection_ray * 20.0));
  vec2 reflected_color_coords = getScreenCoordinates(view_reflection_ray, matProjection);
  vec4 reflection_color_and_depth = texture(texColorAndDepth, reflected_color_coords);
  vec4 sky = getSkyColor(reflection_ray, sunDirection, sunColor, atmosphereColor, altitude);
  vec3 sky_color = sky.rgb;
  float sky_intensity = sky.w;
  vec3 reflection_color = vec3(0);

  #define PI 3.141592
  #define TAU PI * 2.0

  // @hack use altitude to adjust from where we sample the cloud texture,
  // creating the illusion that the far plane represents the horizon line
  float height_above_surface = cameraPosition.y - world_position.y;
  float height_above_surface_reflection_adjustment_factor = 1 - height_above_surface / 100000.0;

  // @todo refactor
  vec2 cloudsUv = vec2(
    -(atan(reflection_ray.z, reflection_ray.x) + PI) / TAU + time * CLOUD_MOVEMENT_RATE,
    -reflection_ray.y + 0.5 - height_above_surface_reflection_adjustment_factor
  );

  // @todo refactor
  vec4 clouds = texture(texClouds, cloudsUv);
  vec3 clouds_color = mix(clouds.rgb, sky_color, 0.3) * clouds.a;

  if (clouds.a > 0.5 && sky_intensity < clouds.a * 0.2) {
    sky_intensity = clouds.a * 0.2;
  }

  if (isOffScreen(reflected_color_coords, 0)) {
    reflection_color = sky_color + clouds_color;
  } else {
    // Fade from geometry reflections to sky reflections depending on
    // how deep into the geometry the reflection ray went. We want a
    // gradual transition from geometry to sky reflections instead of
    // an abrupt cutoff/fallback if no geometry is reflected.
    float depth_distance = getLinearizedDepth(reflection_color_and_depth.w, zNear, zFar) - view_reflection_ray.z;
    float alpha = max(0.0, min(1.0, depth_distance / 20.0));

    reflection_color = mix(reflection_color_and_depth.rgb, sky_color + clouds_color, alpha);
  }

  float plane_fresnel = dot(normalize(fragNormal), normalized_fragment_to_camera);

  // @todo make configurable
  vec3 BASE_OCEAN_COLOR = vec3(0, 0.4 - 0.2 * (1.0 - plane_fresnel), 1.0);

  #if BLOCK_SKYLIGHT_IN_SHADOW == 1
    // Substantially reduce the sky intensity in shadowed areas
    // to avoid erroneously reflecting sunlight/clouds
    float linearized_depth = getLinearizedDepth(gl_FragCoord.z, zNear, zFar);
    vec4 transform = getLightSpaceTransform(matLightViewProjection, world_position);
    float shadow_map_depth = texture(texShadowMap, transform.xy).r;

    // Gracefully fade out fourth-cascade shadows
    float x_distance = 1.0 - 2.0 * distance(transform.x, 0.5);
    float y_distance = 1.0 - 2.0 * distance(transform.y, 0.5);
    float z_distance = 1.0 - 2.0 * distance(transform.z, 0.5);

    float fade_out_factor = pow(1.0 - (x_distance * y_distance * z_distance), 10);

    fade_out_factor = isnan(fade_out_factor) ? 0.0 : fade_out_factor;

    if (transform.z < 0.999 && shadow_map_depth < transform.z - 0.001) {
      sky_intensity *= (0.1 + 0.9 * fade_out_factor);

      BASE_OCEAN_COLOR.xyz *= (1 - vec3(0.8, 0.6, 0.2) * (1 - fade_out_factor));
    }
  #endif

  // Diminish reflections based on sky intensity
  reflection_color = mix(BASE_OCEAN_COLOR, reflection_color, sky_intensity);

  // @hack use the flat plane fresnel, and bias the mix factor
  // toward the reflection color for artistic effect
  ocean_color = mix(refraction_color, reflection_color, pow(1.0 - plane_fresnel, 0.5));

  // @hack Fade to aquamarine at grazing angles
  ocean_color += vec3(0, 1, 1) * pow(1.0 - plane_fresnel, 8);

  // @todo cleanup
  // Apply a simplex noise pattern to modulate the color of the ocean over great distances
  float wx = world_position.x;
  float wz = world_position.z;
  float t = time;
  float s = simplex_noise(vec2(wx * 0.00001, wz * 0.00001));
  float s2 = simplex_noise(vec2(wx * 0.000013, wz * 0.000013));

  ocean_color += vec3(0.1 * s, 0.8 * s, 0.5 * s2) * 0.3;

  ocean_color *= fragColor;

  out_color_and_depth = vec4(ocean_color, gl_FragCoord.z);
}