#version 460 core

uniform sampler2D texColorAndDepth;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

void main() {
  out_color_and_depth = texture(texColorAndDepth, fragUv);
}