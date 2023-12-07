#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 vs_normal;
layout(location = 2) in vec2 in_uv;

uniform mat4 projection;
uniform mat4 view;

struct TreeModel{
    mat4 model;
    mat4 inv_model;
};
layout (std140) uniform Models{
   TreeModel tree_models[500];
};

out vec2 uv;
out vec3 fs_position;
out vec3 normal;

uniform sampler2D noise_texture;

uniform float terrain_height_multiplier;

void main()
{
    mat4 model = tree_models[gl_InstanceID].model;
    mat4 model_inv = tree_models[gl_InstanceID].inv_model;

    float terrain_max = 50;
    float tree_scale = 2.5 * (terrain_height_multiplier/terrain_max);

    uv = in_uv;
    fs_position = vec3(model * position).xyz;
    normal = normalize(model_inv * vec4(vs_normal.xyz, 0.0f)).xyz;

    float height_factor = texture(noise_texture, vec2((model[3][0] + 50.0f)/100.0f, (model[3][2] + 50.0f)/100.0f)).r;

    vec4 terrain_position = vec4(tree_scale * position.xyz, 1.0f);
    terrain_position.y += height_factor * terrain_height_multiplier - terrain_height_multiplier/2.f;

	gl_Position = projection * view * model * terrain_position;
}