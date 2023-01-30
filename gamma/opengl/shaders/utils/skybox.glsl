const float CLOUD_MOVEMENT_RATE = 0.001;

vec4 getSkyColor(vec3 direction, vec3 sunDirection, vec3 sunColor) {
  vec3 adjusted_sun_color = mix(sunColor, vec3(1), 0.5);
  float sun_brightness = 10;
  float altitude = 0.4;

  float y = direction.y + altitude;
  float cos_y = cos(y);

  // @todo allow for custom configuration
  float base_r = 0.8 * pow(0.5 + 0.5 * cos_y * 0.8, 6);
  float base_g = 0.3 * pow(0.5 + 0.5 * cos_y * 0.9, 7);
  float base_b = pow(0.5 + 0.5 * cos_y, 5);

  float daylight_factor = 3.0 * dot(sunDirection, vec3(0, 1, 0));
  vec3 daylight_color = vec3(0.6, 0.8, 1.0);
  vec3 base_skylight = vec3(2 * pow(0.5 * cos_y + 0.5, 50));
  vec3 daylight = (1.0 - dot(direction, vec3(0, 1, 0))) * daylight_color * daylight_factor;
  vec3 skylight = base_skylight + daylight;

  float sun_factor = dot(direction, sunDirection);
  vec3 sunlight = adjusted_sun_color * sun_brightness * pow(max(sun_factor, 0.0), 100);
  vec3 atmosphere = 0.2 * (skylight + sunlight);

  return vec4(
    max(base_r + atmosphere.r, 0),
    max(base_g + atmosphere.g, 0),
    max(base_b + atmosphere.b, 0),
    (0.3 + pow(max(0.0, sun_factor), 50)) / 1.3
  );
}