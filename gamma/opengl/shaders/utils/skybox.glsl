// @todo track shader dependencies as part of hot reloading
// @todo allow for custom configuration
vec3 getSkyColor(vec3 direction) {
  vec3 sunDirection = normalize(vec3(0.3, 0.5, 1.0));
  vec3 sunColor = vec3(0.8, 0.8, 1.0);
  float sunBrightness = 10;
  float altitude = 0.9;

  float y = direction.y + altitude;
  float z = direction.z;

  float base_r = pow(0.5 + 0.5 * cos(y) * 0.8, 6);
  float base_g = 0.3 * pow(0.5 + 0.5 * cos(y) * 0.9, 7);
  float base_b = pow(0.5 + 0.5 * cos(y), 5);

  vec3 skylight = vec3(2 * pow(0.5 * cos(y) + 0.5, 50));
  vec3 sunlight = sunColor * sunBrightness * pow(max(dot(direction, sunDirection), 0.0), 100);
  vec3 atmosphere = 0.2 * (skylight + sunlight);

  return vec3(
    max(base_r + atmosphere.r, 0),
    max(base_g + atmosphere.g, 0),
    max(base_b + atmosphere.b, 0)
  );
}