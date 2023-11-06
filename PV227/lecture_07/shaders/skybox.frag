#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 tex_coord;		// The vertex texture coordinates.
} in_data;

// The cubemap texture to render.
layout (binding = 0) uniform samplerCube cubemap_tex;

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
	// TASK 1: Nothing to do here, just notice how the cube map is sampled.
	final_color = texture(cubemap_tex, in_data.tex_coord);
}