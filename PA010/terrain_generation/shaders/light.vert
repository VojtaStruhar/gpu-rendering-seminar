#version 330

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;


layout(location = 0) in vec3 position;
layout(location = 2) in vec2 in_uv;

uniform sampler2D noise_texture;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	gl_Position = projection * view * model * vec4(position,1.0);
}
