#version 460 core

// @todo when adding support for transparent textures
// in vec2 fragUv;

layout (location = 0) out float depth;

void main() {
  depth = gl_FragCoord.z;
}