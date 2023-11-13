#version 400 core

// The input specification.
layout(triangles, fractional_even_spacing, ccw) in;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in vec3 vertex[];

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec3 p = vertex[0] * gl_TessCoord.x + vertex[1] * gl_TessCoord.y + vertex[2] * gl_TessCoord.z;
	gl_Position = vec4(p, 1.0);
}