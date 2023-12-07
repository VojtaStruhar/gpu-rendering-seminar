#version 330

in vec2 uv;
in vec3 normal;
in vec3 fs_position;

layout(location = 0) out vec4 final_color;

//lights
uniform vec3 eye_position;
uniform vec4 light_position;
uniform vec3 light_ambient_color;
uniform vec3 light_diffuse_color;
uniform vec3 light_specular_color;

//material (ambient, diffuse, specular, shininess)
uniform vec3 material_ambient_color;
uniform vec3 material_diffuse_color;
uniform vec3 material_specular_color;
uniform float material_shininess;

//textures
uniform sampler2D noise_texture;
uniform sampler2D grass_texture;
uniform sampler2D mountain_texture;
uniform sampler2D grass_texture_normals;
uniform sampler2D mountain_texture_normals;

vec3 get_texture_normal(float k, vec4 grass_normal, vec4 mountain_normal, vec3 plane_normal){

	vec3 texture_normal = mix(mountain_normal, grass_normal, k).xyz;

    vec3 tangent_vector;
	vec3 bitangent_vector;
	if (abs(dot(normal, plane_normal)) > 0.5){
		tangent_vector = vec3(1,0,0);
		bitangent_vector = normalize(cross(tangent_vector, normal));
		tangent_vector = normalize(cross(bitangent_vector, normal));
	}
	else
	{
		tangent_vector = normalize(cross(normal, plane_normal));
		bitangent_vector = normalize(cross(tangent_vector, normal));
		tangent_vector = normalize(cross(normal, plane_normal));
	}

	return normalize(tangent_vector * texture_normal.x + bitangent_vector * texture_normal.y + normal * texture_normal.z);
}


vec4 apply_lighting(vec4 texture_color, vec3 texture_normal){
	vec3 light_vector = light_position.xyz - fs_position * light_position.w;
	vec3 L = normalize(light_vector);
	vec3 N = normalize(texture_normal); 
	vec3 E = normalize(eye_position - fs_position);
	vec3 H = normalize(L + E); 

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);

	vec3 ambient = light_ambient_color * texture_color.rgb;
	vec3 diffuse = NdotL * light_diffuse_color * texture_color.rgb;
	vec3 specular = pow(NdotH, material_shininess) * material_specular_color * light_specular_color * texture_color.rgb;

	return vec4(ambient + diffuse + specular, 1.0);
}

void main()
{
    vec3 plane_normal = vec3(0,1,0);
    float grass_tiling = 5;

	float k = abs(dot(normal, plane_normal));
	k = smoothstep(0.1, 0.7, k);

	vec4 grass_normal = normalize(texture(grass_texture_normals, uv * grass_tiling) * 2.0 - 1.0);
	vec4 mountain_normal = normalize(texture(mountain_texture_normals, uv) * 2.0 - 1.0);
	vec4 texture_color = mix(texture(mountain_texture, uv), texture(grass_texture, uv * grass_tiling), k);

	vec3 texture_normal = get_texture_normal(k, grass_normal, mountain_normal, plane_normal);

    final_color = apply_lighting(texture_color.rgba, texture_normal.rgb);
}