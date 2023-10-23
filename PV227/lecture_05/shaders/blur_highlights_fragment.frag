#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture to be blurred.
layout (binding = 0) uniform sampler2D highlights_texture;


// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_blurred_highlight;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 4: Blur input texture. 
	//    Hint: Usefull functions: textureSize, textureLod, max
	//    Hint: We want to spread bright pixel = use max function.
	//	  Hint: What about kernel size? try starting with 5x5

	final_blurred_highlight = vec4(0.0, 0.0, 0.0, 1.0);
}