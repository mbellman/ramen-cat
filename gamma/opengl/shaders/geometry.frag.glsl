#version 460 core

uniform bool hasTexture = false;
uniform bool hasNormalMap = false;
uniform bool useCloseTranslucency = false;
uniform sampler2D meshTexture;
uniform sampler2D meshNormalMap;
uniform float emissivity = 0.0;
uniform float roughness = 0.6;

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_material;

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

  if (color.a < 0.3) {
    discard;
  }

  if (useCloseTranslucency) {
    if (gl_FragCoord.z < 0.95 && int(gl_FragCoord.x) % 3 < 2) {
      discard;
    }
  }

  float material = 0.0;

  material += floor(10 * emissivity);
  material += roughness * 0.99;

  out_color_and_depth = vec4(color.rgb, gl_FragCoord.z);
  out_normal_and_material = vec4(getNormal(), material);
}