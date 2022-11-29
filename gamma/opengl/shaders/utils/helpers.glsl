float saturate(float n) {
  return clamp(n, 0.0, 1.0);
}

bool isOffScreen(vec2 uv, float bias) {
  return uv.x <= bias || uv.x >= 1.0 - bias || uv.y <= bias || uv.y >= 1.0 - bias;
}