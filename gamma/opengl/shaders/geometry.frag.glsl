#version 460 core

uniform bool hasTexture = false;
uniform bool hasNormalMap = false;
uniform sampler2D meshTexture;
uniform sampler2D meshNormalMap;
uniform float meshEmissivity = 0.0;  // @todo material parameters?

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_emissivity;

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
  vec4 color = hasTexture ? texture(meshTexture, fragUv) * vec4(fragColor, 1.0) : vec4(fragColor, 1.0);

  if (color.a < 0.5) {
    discard;
  }

  out_color_and_depth = vec4(color.rgb, gl_FragCoord.z);
  out_normal_and_emissivity = vec4(getNormal(), meshEmissivity);
}