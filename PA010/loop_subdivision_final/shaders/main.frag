#version 330

layout(location = 0) out vec4 final_color;
uniform vec4 input_color;

void main()
{
    final_color = input_color;
}
