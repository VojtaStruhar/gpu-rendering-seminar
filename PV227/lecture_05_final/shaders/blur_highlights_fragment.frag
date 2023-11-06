#version 450 core

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
// The kernel for gaussian blur.
const float kernel[5] = float[5](1.0, 1.0, 1.0, 1.0, 1.0);
const float kernel_sum = 5.0 * 5.0;

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

	// Simple average blur.
	// Size of one texel when texture coordinates are in range (0,1).
	vec2 texel_size = 1.0 / textureSize(highlights_texture, 0);		
	vec3 sum = vec3(0.0);

	for (int x = 0; x < kernel.length(); x++)
	{
		for (int y = 0; y < kernel.length(); y++)
		{
			// Offset from the center texel
			vec2 offset = texel_size * (vec2(x, y) - vec2(kernel.length()/2));

			// Adding weighted value
			sum += kernel[x] * kernel[y] * textureLod(highlights_texture, in_data.tex_coord + offset, 0).rgb;
		}
	}	

	vec3 col = sum / kernel_sum;
	final_blurred_highlight = vec4(max(textureLod(highlights_texture, in_data.tex_coord, 0).rgb, col), 1.0);
}