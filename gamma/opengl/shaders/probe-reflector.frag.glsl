#version 460 core

uniform bool hasTexture = false;
uniform bool hasNormalMap = false;
uniform vec3 cameraPosition;
uniform vec3 probePosition;
uniform sampler2D meshTexture;
uniform sampler2D meshNormalMap;
uniform samplerCube probeMap;

flat in vec3 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_emissivity;

#include "utils/gl.glsl";

vec3 getNormal() {
  vec3 n_fragNormal = normalize(fragNormal);

  if (hasNormalMap) {
    vec3 mappedNormal = texture(meshNormalMap, fragUv).rgb * 2.0 - vec3(1.0);

    mat3 tangentMatrix = mat3(
      normalize(fragTangent),
      normalize(fragBitangent),
      n_fragNormal
    );

    return normalize(tangentMatrix * mappedNormal);
  } else {
    return n_fragNormal;
  }
}

void main() {
  vec3 color = hasTexture ? texture(meshTexture, fragUv).rgb * fragColor : fragColor;
  vec3 normal = getNormal();

  vec3 incident_vector = fragPosition - cameraPosition;
  vec3 reflection_vector = reflect(incident_vector, normal);

  // https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
  // @todo support custom probe reflector bounding boxes
  vec3 box_max = probePosition + vec3(10000.0);
  vec3 box_min = probePosition - vec3(10000.0);

  vec3 first_plane_intersection = (box_max - fragPosition) / reflection_vector;
  vec3 second_plane_intersection = (box_min - fragPosition) / reflection_vector;
  vec3 furthest_plane = max(first_plane_intersection, second_plane_intersection);
  float plane_distance = min(min(furthest_plane.x, furthest_plane.y), furthest_plane.z);
  vec3 intersection_point = fragPosition + reflection_vector * plane_distance;
  vec3 sample_vector = intersection_point - probePosition;

  // @todo support roughness
  vec3 probe_reflector_color = color * texture(probeMap, sample_vector).rgb;

  out_color_and_depth = vec4(probe_reflector_color, gl_FragCoord.z);
  // @todo support custom emissivity
  out_normal_and_emissivity = vec4(normal, 0.3);
}