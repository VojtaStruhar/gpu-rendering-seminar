#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

/** The texture with the lit scene. */
layout (binding = 0) uniform sampler2D lit_scene;
/** The texture with blurred highlights. */
layout (binding = 1) uniform sampler2D blurred_highlights;

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
    // TASK 5: Implement a bloom effect by combining the lit scene with the blurred highlights.
    //   Hint: Do not forget to implement tone mapping.
    //   Hint: When combining the colors from textures you can use simple aditive blending.
    //   Hint: Useful functions: exp, pow

	// Combines lit scene with blurred highlights - simple additive blending.
    vec3 hdrColor = texture(lit_scene, in_data.tex_coord).rgb;      
    hdrColor += texture(blurred_highlights, in_data.tex_coord).rgb; 
    
	// Applies tone mapping.
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
	// Applies gamma correction while we're at it.       
    result = pow(result, vec3(1.0 / gamma));
    final_color = vec4(result, 1.0f);
}