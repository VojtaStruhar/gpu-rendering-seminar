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

void main()
{
	final_color = texture(noise_texture, uv);

}