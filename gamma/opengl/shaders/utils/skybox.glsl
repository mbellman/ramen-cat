// @todo allow for custom configuration
vec4 getSkyColor(vec3 direction) {
  vec3 sun_direction = normalize(vec3(0.2, 1, 1));
  vec3 sun_color = vec3(0.8, 0.8, 0.3);
  float sun_brightness = 10;
  float altitude = 0.4;

  float y = direction.y + altitude;
  float z = direction.z;

  float base_r = 0.8 * pow(0.5 + 0.5 * cos(y) * 0.8, 6);
  float base_g = 0.3 * pow(0.5 + 0.5 * cos(y) * 0.9, 7);
  float base_b = pow(0.5 + 0.5 * cos(y), 5);

  float sun_dot = dot(direction, sun_direction);

  vec3 skylight = vec3(2 * pow(0.5 * cos(y) + 0.5, 50));
  vec3 sunlight = sun_color * sun_brightness * pow(max(sun_dot, 0.0), 100);
  vec3 atmosphere = 0.2 * (skylight + sunlight);

  return vec4(
    max(base_r + atmosphere.r, 0),
    max(base_g + atmosphere.g, 0),
    max(base_b + atmosphere.b, 0),
    (0.3 + pow(max(0.0, sun_dot), 50)) / 1.3
  );
}