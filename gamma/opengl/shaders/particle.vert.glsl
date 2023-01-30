#version 460 core

uniform mat4 matProjection;
uniform mat4 matView;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelColor;
layout (location = 5) in mat4 modelMatrix;

out vec2 fragUv;
flat out vec3 color;

#include "utils/gl.glsl";

// @todo move to utils
vec3 unpack(uint color) {
  float r = float(color & 0x0000FF) / 255.0;
  float g = float((color & 0x00FF00) >> 8) / 255.0;
  float b = float((color & 0xFF0000) >> 16) / 255.0;

  return vec3(r, g, b);
}

void main() {
  float scale = modelMatrix[0][0];

  // @hack invert Z
  vec4 world_position = glVec4(modelMatrix * vec4(vertexPosition, 1.0));

  gl_Position = matProjection * matView * world_position;
  gl_PointSize = 5000.0 * scale / gl_Position.z;

  fragUv = vertexUv;
  color = unpack(modelColor);
}