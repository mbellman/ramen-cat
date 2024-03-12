#version 460 core

uniform sampler2D meshTexture;
uniform mat4 matViewProjection;
uniform bool useXzPlaneTexturing = false;
uniform bool useYPlaneTexturing = false;

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

/**
 * Returns a bitangent from potentially non-orthonormal
 * normal/tangent vectors using the Gram-Schmidt process.
 */
vec3 getFragBitangent(vec3 normal, vec3 tangent) {
  // Redefine the tangent by using the projection of the tangent
  // onto the normal line and defining a vector from that to the
  // original tangent, orthonormalizing the normal/tangent
  tangent = normalize(tangent - dot(tangent, normal) * normal);

  return cross(tangent, normal);
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
  vec2 meshTextureSize = textureSize(meshTexture, 0);
  float planeTexturingDivisor = 400.0 * meshTextureSize.x / 1024.0;

  gl_Position = matViewProjection * world_position;

  fragColor = unpack(modelColor);
  // @hack invert Z
  fragPosition = glVec3(world_position.xyz);
  fragNormal = normal_matrix * vertexNormal;
  fragTangent = normal_matrix * vertexTangent;
  fragBitangent = getFragBitangent(fragNormal, fragTangent);

  if (useXzPlaneTexturing) {
    fragUv = world_position.xz / planeTexturingDivisor;
  } else if (useYPlaneTexturing) {
    
    float dotXf = dot(fragNormal, vec3(1, 0, 0));
    float dotZb = dot(fragNormal, vec3(0, 0, -1));

    // @todo don't hard-code the divisor
    if (dotXf > 0 && dotZb > 0) {
      fragUv = vec2(world_position.x - world_position.z, world_position.y) / 1200.0;      
    } else if (dotXf > 0 || dotZb > 0) {
      fragUv = vec2(world_position.x + world_position.z, world_position.y) / 1200.0;
    } else {
      fragUv = vec2(world_position.x - world_position.z, world_position.y) / 1200.0;
    }
  } else {
    fragUv = vertexUv;
  }
}