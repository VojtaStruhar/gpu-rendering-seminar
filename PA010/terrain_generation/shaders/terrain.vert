#version 330

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


layout(location = 0) in vec3 position;
layout(location = 2) in vec2 in_uv;

out vec2 uv;
out vec3 normal;
out vec3 fs_position;

uniform sampler2D noise_texture;

uniform float terrain_height_multiplier;

void main()
{
    float size = 100.0f;
    float e = 1.0f/size;

    uv = in_uv;
    float mountain_size = terrain_height_multiplier * texture(noise_texture, uv).r;

    float l = texture(noise_texture, vec2(uv.x - e, uv.y)).x;
    float r = texture(noise_texture, vec2(uv.x + e, uv.y)).x;
    
    float t = texture(noise_texture, vec2(uv.x, uv.y - e)).x;
    float b = texture(noise_texture, vec2(uv.x, uv.y + e)).x;

    vec3 T = normalize(vec3(2*e, r-l, 0));
	vec3 B = normalize(vec3(0, b-t, 2*e));
	normal = normalize(cross(B,T));

    gl_Position = projection * view * model * vec4(position.x, mountain_size - terrain_height_multiplier/2.f, position.z, 1.0f);
}