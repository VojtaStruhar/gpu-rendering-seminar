#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

/** The texture to which tone mapping will be applied. */
layout (binding = 0) uniform sampler2D input_tex;
/** The exposure value. */
uniform float exposure;
/** The gamma value. */
uniform float gamma;

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
	// TASK 2: Tone mapping - experiment with different methods.
	//          a) Reinhard mapping
	//          b) Exposure mapping
	//          c) Exposure mapping with gamma correction
	//   Hint: Useful functions: exp2, pow.
	
	final_color = vec4(0.0, 0.0, 0.0, 1.0);
}