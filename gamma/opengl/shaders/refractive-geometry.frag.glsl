#version 460 core

uniform bool hasNormalMap = false;
uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform sampler2D meshNormalMap;
uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform vec3 atmosphereColor;
uniform float altitude;

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

vec3 getNormal() {
  vec3 normalized_frag_normal = normalize(fragNormal);

  if (hasNormalMap) {
    vec3 mappedNormal = texture(meshNormalMap, fragUv).rgb * 2.0 - vec3(1.0);

    mat3 tangentMatrix = mat3(
      normalize(fragTangent),
      normalize(fragBitangent),
      normalized_frag_normal
    );

    return normalize(tangentMatrix * mappedNormal);
  } else {
    return normalized_frag_normal;
  }
}

void main() {
  const float REFRACTION_INTENSITY = 4.0;

  vec3 position = getWorldPosition(gl_FragCoord.z, getPixelCoords(), matInverseProjection, matInverseView);
  vec3 color = vec3(1.0);
  vec3 normal = getNormal();
  vec3 normalized_fragment_to_camera = normalize(cameraPosition - position);

  vec3 refraction_ray = refract(normalized_fragment_to_camera, normal, 0.7);
  vec3 world_refraction_ray = position + refraction_ray * REFRACTION_INTENSITY;
  // @hack invert Z
  vec3 view_refraction_ray = glVec3(matView * glVec4(world_refraction_ray));
  vec2 refracted_color_coords = getScreenCoordinates(view_refraction_ray, matProjection);
  float sample_depth = texture(texColorAndDepth, getPixelCoords()).w;

  if (sample_depth < 1.0 && isOffScreen(refracted_color_coords, 0.0)) {
    // If the fragment has a depth closer than the far plane,
    // discard any attempts at offscreen color reading
    discard;
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

  if (refracted_color_and_depth.w == 1.0) {
    // Skybox
    vec3 sky_direction = normalize(world_refraction_ray - cameraPosition);

    refracted_color_and_depth.rgb = getSkyColor(sky_direction, sunDirection, sunColor, atmosphereColor, altitude).rgb;
  }

  // Slightly darken fragments facing the camera more directly
  float intensity = 1.0 - 0.2 * dot(normal, normalized_fragment_to_camera);

  refracted_color_and_depth.rgb *= fragColor;

  out_color_and_depth = vec4(refracted_color_and_depth.rgb * intensity, gl_FragCoord.z);
}