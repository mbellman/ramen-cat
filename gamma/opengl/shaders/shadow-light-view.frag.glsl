#version 460 core

uniform bool hasTexture = false;
uniform sampler2D meshTexture;

in vec2 fragUv;

layout (location = 0) out float depth;

void main() {
  if (hasTexture && texture(meshTexture, fragUv).w < 0.5) {
    discard;
  }

  depth = gl_FragCoord.z;
}