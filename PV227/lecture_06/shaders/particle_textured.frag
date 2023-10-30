#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 color;	       // The particle color.
	vec2 tex_coord;    // The texture coordinates for the particle.
} in_data;

// The particle texture.
layout (binding = 0) uniform sampler2D particle_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final fragment color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float intensity = texture(particle_texture, in_data.tex_coord).r;
	final_color = vec4(in_data.color * intensity, intensity);
}