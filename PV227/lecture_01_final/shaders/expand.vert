#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
layout (location = 0) in vec4 position;  // The vertex position.
layout (location = 1) in vec3 normal;    // The vertex normal.
layout (location = 2) in vec2 tex_coord; // The vertex texture coordinates.
	
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
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	out_data.tex_coord = tex_coord;
	out_data.position_ws = vec3(model * position);
	out_data.normal_ws = normalize(model_it * normal);

	// Solution #1 (prefered) using normal of the object.
	// - problems can occur for nonconvex objects if the shift would be too big 
	// - works well for (convex) objects with holes (e.g., torus)
	vec4 pos = position + vec4(normal, 0.0) * 0.02;
	gl_Position = projection * view * model * pos;

	// Solution #2 using the scaling from the center of the object. 
	// - problems can occur for objects placed outside (0,0,0) in their local coordinate system
	// - does not work well for objects with holes (e.g., torus)
	//float scale = 1.02;
	//gl_Position = projection * view * model * (vec4(scale,scale,scale,1) * position);
}
