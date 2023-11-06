#version 450 core

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
// The kernel for gaussian blur.
const float gauss9[9] = float[9](1.0, 8.0, 28.0, 56.0, 70.0, 56.0, 28.0, 8.0, 1.0);
const float gauss9x9_sum = 256.0 * 256.0;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture with positoins in view space.
layout (binding = 0) uniform sampler2D positions_vs_tex;
// The texture with rendere scene cotaining the lightning and ssao.
layout (binding = 1) uniform sampler2D color_tex;

// The distance of objects in focus.
uniform int focus_distance;

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
	// Width of the blur: 0.0 - no blur, 1.0 - max blur
	float blur_width = 1.0;

	// Computes the width of the blur by comparing the difference between the distance of the object from the camera and the focus distance. 
	vec4 position_vs = texture(positions_vs_tex, in_data.tex_coord);
	if (position_vs.w != 0.0)
	{
		float object_distance = length(position_vs.xyz);
		blur_width = min(10.0, abs(focus_distance - object_distance)) / 10.0;
	}

	// Performs Gaussian blur.
	// Size of one texel when texture coordinates are in range (0,1).
	vec2 texel_size = 1.0 / textureSize(color_tex, 0);		
	vec3 sum = vec3(0.0);
	for (int x = 0; x < gauss9.length(); x++)
	for (int y = 0; y < gauss9.length(); y++)
	{
		// Offset from the center texel.
		vec2 offset = texel_size * (vec2(x, y) - vec2(gauss9.length()/2));
		// Adding weighted value.
		sum += gauss9[x] * gauss9[y] * textureLod(color_tex, in_data.tex_coord + offset * blur_width, 0).xyz;
	}
	final_color = vec4(sum / gauss9x9_sum, 0.0);
}