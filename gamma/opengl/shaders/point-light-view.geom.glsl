#version 460 core

uniform mat4 lightMatrices[6];

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

out vec4 world_position;

void main() {
  for (int f = 0; f < 6; f++) {
    gl_Layer = f;

    for (int v = 0; v < 3; v++) {
      world_position = gl_in[v].gl_Position;
      gl_Position = lightMatrices[f] * world_position;

      EmitVertex();
    }
    
    EndPrimitive();
  }
}  