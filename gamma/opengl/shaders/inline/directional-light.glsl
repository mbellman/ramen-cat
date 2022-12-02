vec3 adjusted_light_color = light.color * light.power;
// @optimize normalize light direction outside of the shader
vec3 normalized_surface_to_light = normalize(light.direction) * -1.0;
vec3 normalized_surface_to_camera = normalize(cameraPosition - position);
vec3 half_vector = normalize(normalized_surface_to_light + normalized_surface_to_camera);
float incidence = max(dot(normalized_surface_to_light, normal), 0.0);
float specularity = pow(max(dot(half_vector, normal), 0.0), 50) * (1.0 - roughness);

vec3 diffuse_term = color * adjusted_light_color * incidence * sqrt(roughness) * (1.0 - specularity);
vec3 specular_term = adjusted_light_color * specularity;

vec3 illuminated_color = diffuse_term + specular_term;