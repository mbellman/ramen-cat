#version 460 core

#define USE_DEPTH_OF_FIELD 0

uniform sampler2D texColorAndDepth;

uniform float zNear;
uniform float zFar;

in vec2 fragUv;

layout (location = 0) out vec3 out_color;

#include "utils/random.glsl";
#include "utils/conversion.glsl";

/**
 * Applies a noise filter to colors to reduce banding
 * artifacts, strengthening the effect in darker ranges
 * where banding is more apparent.
 */
vec3 deband(vec3 color) {
  float brightness = max(color.r, 0) + max(color.g, 0) + max(color.b, 0);
  float divisor = brightness * 150.0;

  return color * (1.0 + random(0.0, 1.0) / divisor);
}

void main() {
  #if USE_DEPTH_OF_FIELD == 1
    const int MIP_LEVEL = 1;
    const float MAX_DEPTH = 3000.0;

    vec2 texelSize = 1.0 / textureSize(texColorAndDepth, MIP_LEVEL);

    vec3 depth_of_field_color = vec3(0.0);

    vec2 uv1 = fragUv + vec2(-1.0, 0.0) * texelSize;
    vec2 uv2 = fragUv + vec2(1.0, 0.0) * texelSize;
    vec2 uv3 = fragUv + vec2(0.0, -1.0) * texelSize;
    vec2 uv4 = fragUv + vec2(0.0, 1.0) * texelSize;

    depth_of_field_color += textureLod(texColorAndDepth, fragUv, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv1, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv2, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv3, MIP_LEVEL).rgb;
    depth_of_field_color += textureLod(texColorAndDepth, uv4, MIP_LEVEL).rgb;
    depth_of_field_color /= 5.0;

    vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
    float depth_factor = getLinearizedDepth(frag_color_and_depth.w, zNear, zFar) / MAX_DEPTH;

    if (depth_factor > 1.0) depth_factor = 1.0;

    depth_factor *= depth_factor;

    out_color = mix(frag_color_and_depth.rgb, depth_of_field_color, depth_factor);
  #else
    out_color = deband(texture(texColorAndDepth, fragUv).rgb);
  #endif
}