#version 460 core

uniform sampler2D texCascade0;
uniform sampler2D texCascade1;
uniform sampler2D texCascade2;

noperspective in vec2 fragUv;

out vec3 out_color;

const float one_third = 1.0 / 3.0;
const float two_thirds = 2.0 / 3.0;

void main() {
  if (fragUv.x < one_third) {
    vec2 sampleUv = fragUv * vec2(3.0, 1.0);
    float depth = texture(texCascade0, sampleUv).r;

    out_color = vec3(depth);
  } else if (fragUv.x < two_thirds) {
    vec2 sampleUv = (fragUv - vec2(one_third, 0.0)) * vec2(3.0, 1.0);
    float depth = texture(texCascade1, sampleUv).r;

    out_color = vec3(depth);
  } else {
    vec2 sampleUv = (fragUv - vec2(two_thirds, 0.0)) * vec2(3.0, 1.0);
    float depth = texture(texCascade2, sampleUv).r;

    out_color = vec3(depth);
  }
}