#version 460 core

uniform mat4 matProjection;
uniform mat4 matView;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexTangent;
layout (location = 3) in vec2 vertexUv;
layout (location = 4) in uint modelColor;
layout (location = 5) in mat4 modelMatrix;

flat out vec3 fragColor;
out vec3 fragPosition;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;

#include "utils/gl.glsl";
#include "utils/foliage.glsl";

/**
 * Returns a bitangent from potentially non-orthonormal
 * normal/tangent vectors using the Gram-Schmidt process.
 */
vec3 getFragBitangent(vec3 normal, vec3 tangent) {
  vec3 n_normal = normalize(normal);
  vec3 n_tangent = normalize(tangent);

  // Redefine the tangent by using the projection of the tangent
  // onto the normal line and defining a vector from that to the
  // original tangent, orthonormalizing the normal/tangent
  n_tangent = normalize(n_tangent - dot(n_tangent, n_normal) * n_normal);

  return cross(n_tangent, n_normal);
}

// @todo move to utils
vec3 unpack(uint color) {
  float r = float(color & 0x0000FF) / 255.0;
  float g = float((color & 0x00FF00) >> 8) / 255.0;
  float b = float((color & 0xFF0000) >> 16) / 255.0;

  return vec3(r, g, b);
}

void main() {
  // @hack invert Z
  vec4 world_position = glVec4(modelMatrix * vec4(vertexPosition, 1.0));
  mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));

  // @todo make a utility for this
  switch (foliage.type) {
    case FLOWER:
      world_position.xyz += getFlowerFoliageOffset(world_position.xyz);
      break;
    case BRANCH:
      world_position.xyz += getBranchFoliageOffset(world_position.xyz);
      break;
    case LEAF:
      world_position.xyz += getLeafFoliageOffset(world_position.xyz);
      break;
  }

  gl_Position = matProjection * matView * world_position;

  fragColor = unpack(modelColor);
  // @hack invert Z
  fragPosition = glVec3(world_position.xyz);
  fragNormal = normal_matrix * vertexNormal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);
  fragUv = vertexUv;
}