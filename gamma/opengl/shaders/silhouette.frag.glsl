#version 460 core

uniform bool hasTexture = false;
uniform sampler2D meshTexture;

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  if (hasTexture && texture(meshTexture, fragUv).a < 0.5) {
    discard;
  }

  // @todo make configurable
  const vec3 SILHOUETTE_COLOR = vec3(1);

  out_color_and_depth = vec4(SILHOUETTE_COLOR, 1.0);
}