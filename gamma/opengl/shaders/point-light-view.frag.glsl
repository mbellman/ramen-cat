#version 460 core

uniform vec3 lightPosition;
uniform float farPlane;

in vec4 world_position;

void main() {
  gl_FragDepth = length(world_position.xyz - lightPosition) / farPlane;
}