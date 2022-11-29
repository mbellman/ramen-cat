vec3 glVec3(vec3 vector) {
  return vector * vec3(1, 1, -1);
}

vec3 glVec3(vec4 vector) {
  return vector.xyz * vec3(1, 1, -1);
}

vec4 glVec4(vec4 vector) {
  return vector * vec4(1, 1, -1, 1);
}

vec4 glVec4(vec3 vector) {
  return vec4(glVec3(vector), 1.0);
}