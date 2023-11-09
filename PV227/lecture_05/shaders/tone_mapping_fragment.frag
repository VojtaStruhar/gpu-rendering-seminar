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

	vec3 hdrColor = texture(input_tex, in_data.tex_coord).rgb;
	vec3 result;

	// a)
//	result = hdrColor / (hdrColor + vec3(1.0f));

	// b)
	result = vec3(1.0) - exp2(-hdrColor * exposure);

	// c)
	vec3 mapped = pow(result, vec3(1.0 / gamma));

	final_color = vec4(mapped, 1.0);
}