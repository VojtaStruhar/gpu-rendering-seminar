#version 450 core

layout (location = 0) out vec4 final_color;

uniform vec3 write_color;

void main()
{
    /// Super simple shader - just writes what its told to write. Controlled from the CPU side.
    final_color = vec4(write_color, 1.0);
}