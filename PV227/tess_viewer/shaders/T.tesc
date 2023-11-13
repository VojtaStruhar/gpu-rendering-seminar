#version 400 core

// The output specification.
layout(vertices = 3) out;

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
const vec3 vertices[3] = vec3[3](
	vec3( 0.5, -0.5, 0.0),
	vec3( 0.0,  0.5, 0.0),
	vec3(-0.5, -0.5, 0.0));

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out vec3 vertex[];

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
uniform float TessLevelInner[2];
uniform float TessLevelOuter[4];

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vertex[gl_InvocationID] = vertices[gl_InvocationID];

	gl_TessLevelInner[0] = TessLevelInner[0];
	
	gl_TessLevelOuter[0] = TessLevelOuter[0];
	gl_TessLevelOuter[1] = TessLevelOuter[1];
	gl_TessLevelOuter[2] = TessLevelOuter[2];
}