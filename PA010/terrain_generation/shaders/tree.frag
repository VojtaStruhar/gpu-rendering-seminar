#version 330

in vec2 uv;
in vec3 normal;
in vec3 fs_position;

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

layout(location = 0) out vec4 final_color;
uniform sampler2D tree_texture;


vec4 apply_lighting(vec4 texture_color, vec3 texture_normal){
	vec3 light_vector = light_position.xyz - fs_position * light_position.w;
	vec3 L = normalize(light_vector);
	vec3 N = normalize(texture_normal); 
	vec3 E = normalize(eye_position - fs_position);	
	vec3 H = normalize(L + E);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);

	vec3 ambient = light_ambient_color * texture_color.rgb;
	vec3 diffuse = light_diffuse_color * texture_color.rgb;
	vec3 specular = material_specular_color * light_specular_color * texture_color.rgb;

	vec3 newcolor = ambient + NdotL * diffuse + pow(NdotH, material_shininess) * specular;

	float distance2 = light_position.w == 1.0 ? length(light_vector)*0.005 : 1.0;
	return vec4(newcolor/distance2, texture_color.a);
}

void main()
{
	vec4 texture_color = texture(tree_texture, uv);
    final_color = apply_lighting(texture_color, normal);
	if (texture_color.a == 0){
		discard;
	}
}
