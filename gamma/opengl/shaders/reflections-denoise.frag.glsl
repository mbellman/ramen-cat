#version 460 core

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

float luminance(vec3 color) {
  return length(color * vec3(0.2126, 0.7152, 0.0722));
}

// @todo improve denoising quality; as of right now this
// acts more as a crude selective blur filter
void main() {
  vec2 texel_size = 1.0 / screenSize;

  vec4 frag = texture(texColorAndDepth, fragUv);
  vec4 top = texture(texColorAndDepth, fragUv + texel_size * vec2(0.0, 1.0));
  vec4 bottom = texture(texColorAndDepth, fragUv + texel_size * vec2(0.0, -1.0));
  vec4 left = texture(texColorAndDepth, fragUv + texel_size * vec2(-1.0, 0.0));
  vec4 right = texture(texColorAndDepth, fragUv + texel_size * vec2(1.0, 0.0));

  float frag_luminance = luminance(frag.rgb);
  float top_luminance = luminance(top.rgb);
  float bottom_luminance = luminance(bottom.rgb);
  float left_luminance = luminance(left.rgb);
  float right_luminance = luminance(right.rgb);

  float average_local_luminance = (top_luminance + bottom_luminance + left_luminance + right_luminance) / 4.0;

  if (length(frag_luminance - average_local_luminance) > 0.05) {
    vec3 average = (top.rgb + bottom.rgb + left.rgb + right.rgb) / 4.0;

    out_color_and_depth = vec4(average, frag.w);
  } else {
    out_color_and_depth = vec4(frag.rgb, frag.w);
  }
}