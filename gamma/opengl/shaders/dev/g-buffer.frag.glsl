#version 460 core

uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform float zNear;
uniform float zFar;

noperspective in vec2 fragUv;

out vec3 out_color;

#include "utils/conversion.glsl";

void main() {
  if (fragUv.x < 0.25) {
    // Albedo
    vec2 uv = fragUv * vec2(4.0, 1.0);
    vec4 color_and_depth = texture(texColorAndDepth, uv);

    out_color = color_and_depth.rgb;
  } else if (fragUv.x < 0.5) {
    // Depth (adjusted for clarity)
    vec2 uv = (fragUv - vec2(0.25, 0.0)) * vec2(4.0, 1.0);
    vec4 color_and_depth = texture(texColorAndDepth, uv);
    float adjusted_depth = pow(getLinearizedDepth(color_and_depth.w, zNear, zFar) / zFar, 1.0 / 4.0);

    out_color = vec3(adjusted_depth);
  } else if (fragUv.x < 0.75) {
    // Normal
    vec2 uv = (fragUv - vec2(0.5, 0.0)) * vec2(4.0, 1.0);
    vec4 normal_and_emissivity = texture(texNormalAndEmissivity, uv);

    out_color = normal_and_emissivity.rgb;
  } else {
    // Emissivity
    vec2 uv = (fragUv - vec2(0.75, 0.0)) * vec2(4.0, 1.0);
    vec4 normal_and_emissivity = texture(texNormalAndEmissivity, uv);

    out_color = vec3(normal_and_emissivity.w);
  }
}