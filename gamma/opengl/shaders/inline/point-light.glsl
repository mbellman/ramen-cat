vec4 frag_color_and_depth = texture(texColorAndDepth, fragUv);
vec3 position = getWorldPosition(frag_color_and_depth.w, fragUv, matInverseProjection, matInverseView);
vec3 surface_to_light = light.position - position;
float light_distance = length(surface_to_light);

if (light_distance > light.radius) {
  discard;
}

vec4 frag_normal_and_emissivity = texture(texNormalAndEmissivity, fragUv);
vec3 normal = frag_normal_and_emissivity.xyz;
vec3 color = frag_color_and_depth.rgb;

vec3 normalized_surface_to_light = surface_to_light / light_distance;
vec3 normalized_surface_to_camera = normalize(cameraPosition - position);
vec3 half_vector = normalize(normalized_surface_to_light + normalized_surface_to_camera);
float incidence = max(dot(normalized_surface_to_light, normal), 0.0);
float attenuation = pow(1.0 / light_distance, 2);
float specularity = pow(max(dot(half_vector, normal), 0.0), 50);

// Define a non-linear light intensity fall-off toward the radius boundary
float hack_diffuse_radial_influence = (1.0 - pow(clamp(light_distance / light.radius, 0.0, 1.0), 2));
float hack_specular_radial_influence = (1.0 - pow(clamp(light_distance / light.radius, 0.0, 1.0), 10));
// Taper light intensity more softly to preserve light with distance
float hack_soft_tapering = (20.0 * (light_distance / light.radius));

vec3 radiant_flux = light.color * light.power * light.radius;
vec3 diffuse_term = color * radiant_flux * incidence * attenuation * hack_diffuse_radial_influence * hack_soft_tapering * (1.0 - specularity);
vec3 specular_term = 5.0 * radiant_flux * specularity * attenuation * hack_specular_radial_influence;

float emissivity = frag_normal_and_emissivity.w;
vec3 illuminated_color = (diffuse_term + specular_term) * (1.0 - emissivity);