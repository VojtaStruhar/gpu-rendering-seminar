#version 450 core

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
// Texture coordinates for the full screen quad.
const vec2 tex_coords[3] = vec2[3] (
	vec2(0.0, 0.0),
	vec2(2.0, 0.0),
	vec2(0.0, 2.0)
);

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;		// The projection matrix.
	mat4 projection_inv;	// The inverse of the projection matrix.
	mat4 view;				// The view matrix
	mat4 view_inv;			// The inverse of the view matrix.
	mat3 view_it;			// The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// The position of the eye in world space.
};

// The UBO with the model data.
layout (std140, binding = 1) uniform ModelData
{
	mat4 model;			// The model matrix.
	mat4 model_inv;		// The inverse of the model matrix.
	mat3 model_it;		// The inverse of the transpose of the top-left part 3x3 of the model matrix.
};

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	vec3 tex_coord;		// The vertex texture coordinates.
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// gl_VertexID holds the index of the vertex that is being processed.
	vec2 tex_coord = tex_coords[gl_VertexID];

	// We compute the view direction based on the position of the vertex in clip space.
	vec4 pos_cs = vec4(tex_coord * 2.0 - 1.0, 0.0, 1.0);	// The position of the vertex in clip space.
	vec4 pos_vs = projection_inv * pos_cs;					// We use inverse of the projection matrix to transform the position into view space.
	vec3 dir_vs = pos_vs.xyz;								// In view space, the position of the vertex corresponds to the direction of the view.
	vec3 dir_ws = mat3(view_inv) * dir_vs;					// We transform the direction into the world space.
	out_data.tex_coord = dir_ws;							// The direction in world space is the texture coordinate for the cube map.

	gl_Position = pos_cs;
}