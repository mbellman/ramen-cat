const float CLOUD_MOVEMENT_RATE = 0.001;

vec4 getSkyColor(vec3 sky_direction, vec3 sun_direction, vec3 sun_color, vec3 atmosphere_color, float altitude) {
  float sun_brightness = 1.5;
  float altitude_y_offset = altitude / 50000.0;

  float y = sky_direction.y + altitude_y_offset;
  float cos_y = cos(y);

  // @todo make configurable
  vec3 BASE_DAYLIGHT_COLOR = vec3(0.25, 0.35, 1.0);
  vec3 BASE_NIGHTTIME_COLOR = vec3(0.1, 0.2, 0.6);

  float daylight_factor = dot(sun_direction, vec3(0, 1, 0));
  float altitude_factor = pow(cos_y, 3);
  vec3 daylight = mix(BASE_NIGHTTIME_COLOR, BASE_DAYLIGHT_COLOR, daylight_factor) * altitude_factor;

  float sun_radius_factor = dot(sky_direction, sun_direction);
  vec3 sunlight = sun_color * sun_brightness * pow(max(sun_radius_factor, 0.0), 100);

  vec3 atmosphere = atmosphere_color * pow(cos_y, 100);

  vec3 sky_color = daylight + atmosphere + sunlight;
  float sky_brightness = (0.3 + pow(max(0.0, sun_radius_factor), 50)) / 1.3;

  return vec4(sky_color, sky_brightness);
}