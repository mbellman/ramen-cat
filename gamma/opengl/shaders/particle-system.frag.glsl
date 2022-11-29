#version 460 core

in vec2 fragUv;
in vec3 color;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_emissivity;

void main() {
  float radius = length(gl_PointCoord - vec2(0.5));

  if (radius > 0.5) {
    discard;
  }

  float radius_factor = 1.0 - radius / 0.5;

  out_color_and_depth = vec4(color * radius_factor, gl_FragCoord.z);
  out_normal_and_emissivity = vec4(vec3(0), 1);
}