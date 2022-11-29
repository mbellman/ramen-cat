#version 460 core

#define USE_COMPOSITED_INDIRECT_LIGHT 1

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform sampler2D texNormalAndEmissivity;
uniform sampler2D texIndirectLight;

noperspective in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/random.glsl";
#include "utils/helpers.glsl";
#include "utils/conversion.glsl";

void main() {
  vec4 frag_normal_and_emissivity = texture(texNormalAndEmissivity, fragUv);
  vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
  vec3 fragment_albedo = frag_color_and_depth.rgb;
  float linear_fragment_depth = getLinearizedDepth(frag_color_and_depth.w);
  vec3 fragment_normal = frag_normal_and_emissivity.xyz;
  float emissivity = frag_normal_and_emissivity.w;
  vec3 global_illumination = vec3(0);
  float ambient_occlusion = 0.0;

  #if USE_COMPOSITED_INDIRECT_LIGHT == 1
    vec4 indirect_light = texture(texIndirectLight, fragUv);

    global_illumination = indirect_light.rgb;
    ambient_occlusion = indirect_light.w;
  #endif

  vec3 composite_color = fragment_albedo * emissivity + global_illumination;

  // @bug this tints occluded regions correctly, but
  // produces excessive darkening in certain areas
  composite_color -= ambient_occlusion;

  out_color_and_depth = vec4(composite_color, frag_color_and_depth.w);
}