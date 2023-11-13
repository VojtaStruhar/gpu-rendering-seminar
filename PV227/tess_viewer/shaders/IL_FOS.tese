#version 400 core

// The input specification.
layout(isolines, fractional_odd_spacing) in;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in vec3 vertex[];

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec3 px0 = mix(vertex[0], vertex[1], gl_TessCoord.x);
	vec3 px1 = mix(vertex[3], vertex[2], gl_TessCoord.x);
	vec3 p = mix(px0, px1, gl_TessCoord.y);
	gl_Position = vec4(p, 1.0);
}