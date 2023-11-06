#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The transformation to be applied to the colors of the texture.
uniform mat4 transformation;
// The texture to display.
layout (binding = 0) uniform sampler2D input_tex;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// We applie the transformation to the color from the texture using the uniform matrix.
	final_color = clamp(transformation * texture(input_tex, in_data.tex_coord), vec4(0.0), vec4(1.0));
	//final_color = texture(input_tex, in_data.tex_coord);
}