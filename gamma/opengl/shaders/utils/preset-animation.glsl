#define NONE 0
#define FLOWER 1
#define LEAF 2
#define BIRD 3
#define CLOTH 4
#define NPC 5

struct PresetAnimation {
  int type;
  float speed;
  float factor;
};

uniform PresetAnimation animation;
uniform float time;

vec3 getFlowerAnimationOffset(vec3 vertex_position, vec3 world_position) {
  float rate = time * animation.speed;

  float vertex_distance_from_ground = abs(vertex_position.y * 2.0);
  float x_3 = world_position.x / 3.0;
  float z_3 = world_position.z / 3.0;
  float displacement_factor = animation.factor * min(1.0, pow(vertex_distance_from_ground / 1.5, 2));

  // @todo allow some of these magic numbers to be configurable
  float offset_x = sin(rate + x_3 + z_3) + sin(rate * 3.1 + z_3) * 0.2 + cos(rate * 4.0 + x_3 + z_3) * 0.1;
  float offset_y = 0.0;
  float offset_z = sin(rate + x_3 + z_3) + cos(rate * 1.3 + x_3) * 0.2 + sin(rate * 4.0 + x_3 + z_3) * 0.1;

  return displacement_factor * vec3(offset_x, offset_y, offset_z);
}

vec3 getLeafAnimationOffset(vec3 vertex_position, vec3 world_position) {
  float rate = time * animation.speed;

  float distance_factor = length(vertex_position);// pow(vy / (1.0 + vy), 2);
  float displacement_factor = animation.factor * distance_factor * 2.0;
  float angle = atan(vertex_position.x, vertex_position.z);
  float y_offset = displacement_factor * (sin(rate + gl_InstanceID) + sin(rate + angle * 1.7));

  return vec3(0, y_offset, 0);
}

vec3 getBirdAnimationOffset(vec3 vertex_position, vec3 world_position) {
  float rate = time * animation.speed;
  float id = float(gl_InstanceID);

  float displacement_factor = animation.factor * min(1.0, pow(abs(vertex_position.x), 2)) * 35.0;
  float y_offset = pow(abs(sin(rate)), 2) * sin(rate * 6.0 + id);

  return vec3(0, displacement_factor * y_offset, 0);
}

vec3 getClothAnimationOffset(vec3 vertex_position, vec3 world_position) {
  float rate = 3.0 * time * animation.speed;

  float scale = modelMatrix[1][1];

  float displacement_factor = (1.0 + scale / 2000.0) * animation.factor * sqrt(abs(vertex_position.y));
  float x = 2.0 * sin(rate + vertex_position.y * 10.0 + world_position.x * 0.05);
  float z = cos(rate + vertex_position.y * 10.0 + world_position.x * 0.05);

  return vec3(x, 0, z) * displacement_factor;
}

vec3 getNpcAnimationOffset(vec3 vertex_position, vec3 world_position) {
  float rate = 5.0 * time * animation.speed;

  float displacement_factor = 0.3 * (vertex_position.y - (-1.0));
  float x = vertex_position.x * sin(rate + world_position.x / 100.0);
  float y = sin(rate + world_position.y / 300.0 + world_position.x / 100.0);
  float z = vertex_position.z * sin(rate + world_position.x / 100.0);

  return vec3(x, y, z) * displacement_factor;
}