#version 460 core

struct Light {
  vec3 position;
  float radius;
  vec3 color;
  float power;
  vec3 direction;
  float fov;
};

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 discOffset;
layout (location = 2) in vec2 discScale;
layout (location = 3) in vec3 lightPosition;
layout (location = 4) in float lightRadius;
layout (location = 5) in vec3 lightColor;
layout (location = 6) in float lightPower;
layout (location = 7) in vec3 lightDirection;
layout (location = 8) in float lightFov;

noperspective out vec2 fragUv;
flat out Light light;

void main() {
  gl_Position = vec4(vertexPosition * discScale + discOffset, 0.0, 1.0);

  fragUv = gl_Position.xy * 0.5 + 0.5;
  light = Light(lightPosition, lightRadius, lightColor, lightPower, lightDirection, lightFov);
}