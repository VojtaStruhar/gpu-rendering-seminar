#version 330

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

layout(location = 0) in vec3 position;
uniform float zshift;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    gl_Position.z += zshift;
}
