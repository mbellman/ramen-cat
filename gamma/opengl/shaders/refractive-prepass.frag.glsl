#version 460 core

uniform vec2 screenSize;
uniform mat4 matProjection;
uniform sampler2D texColorAndDepth;

layout (location = 2) out vec4 out_color_and_depth;

#include "utils/conversion.glsl";

vec2 getPixelCoords() {
  return gl_FragCoord.xy / screenSize;
}

void main() {
  float linearized_depth = getLinearizedDepth(gl_FragCoord.z);
  vec4 base_color = texture(texColorAndDepth, getPixelCoords());

  out_color_and_depth = vec4(vec3(1, 0, 1) * base_color.rgb, gl_FragCoord.z);
  gl_FragDepth = getFragDepth(linearized_depth + 1.0, matProjection);
}