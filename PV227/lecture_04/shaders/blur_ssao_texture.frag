#version 450 core

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
// The kernel for gaussian blur.
const float gauss5[5] = float[5](1.0, 4.0, 6.0, 4.0, 1.0);
const float gauss5x5_sum = 16.0 * 16.0;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The SSAO texture to be blurred.
layout (binding = 0) uniform sampler2D ssao_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The blured SSAO values.
layout (location = 0) out float final_blurred_ssao;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// The size of one texel when texture coordinates are in range (0,1).
	vec2 texel_size = 1.0 / textureSize(ssao_texture, 0);		
	
	// We use Gaussian blur.
	float sum = 0.0;
	for (int x = 0; x < gauss5.length(); x++)
	for (int y = 0; y < gauss5.length(); y++)
	{
		// Offset from the center texel.
		vec2 offset = texel_size * (vec2(x, y) - vec2(gauss5.length()/2));
		// Adding weighted value.
		sum += gauss5[x] * gauss5[y] * textureLod(ssao_texture, in_data.tex_coord + offset, 0).r;
	}
	final_blurred_ssao = sum / gauss5x5_sum;
}