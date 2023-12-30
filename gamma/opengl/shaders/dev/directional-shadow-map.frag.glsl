#version 460 core

uniform sampler2D texCascade0;
uniform sampler2D texCascade1;
uniform sampler2D texCascade2;
uniform sampler2D texCascade3;

noperspective in vec2 fragUv;

out vec3 out_color;

const float one_quarter = 1.0 / 4.0;
const float two_quarters = 2.0 / 4.0;
const float three_quarters = 3.0 / 4.0;

void main() {
  if (fragUv.x < one_quarter) {
    vec2 sampleUv = fragUv * vec2(3.0, 1.0);
    float depth = texture(texCascade0, sampleUv).r;

    out_color = vec3(depth);
  } else if (fragUv.x < two_quarters) {
    vec2 sampleUv = (fragUv - vec2(one_quarter, 0.0)) * vec2(3.0, 1.0);
    float depth = texture(texCascade1, sampleUv).r;

    out_color = vec3(depth);
  } else if (fragUv.x < three_quarters) {
    vec2 sampleUv = (fragUv - vec2(two_quarters, 0.0)) * vec2(3.0, 1.0);
    float depth = texture(texCascade2, sampleUv).r;

    out_color = vec3(depth);
  } else {
    vec2 sampleUv = (fragUv - vec2(three_quarters, 0.0)) * vec2(3.0, 1.0);
    float depth = texture(texCascade3, sampleUv).r;

    out_color = vec3(depth);
  }
}