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
	
	// Extracts the color from the texture.
	vec3 hdrColor = texture(input_tex, in_data.tex_coord).rgb;

	// TASK 2a - Reinhard mapping
	//vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

	// TASK 2b - Exposure mapping
    vec3 mapped = vec3(1.0f) - exp2(-hdrColor * exposure);
	// TASK 2c Exposure mapping with gamma correction.
    mapped = pow(mapped, vec3(1.0 / gamma));

    final_color = vec4(mapped, 1.0);
}