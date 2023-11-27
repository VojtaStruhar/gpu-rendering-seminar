#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
layout (location = 0) in vec4 position;  // The vertex position.
layout (location = 1) in vec3 normal;	 // The vertex normal.
layout (location = 2) in vec2 tex_coord; // The vertex texture coordinates.
layout (location = 3) in vec3 tangent;   // The vertex tangent.
layout (location = 4) in vec3 bitangent; // The vertex bitangent.

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
	vec3 tangent_ws;	  // The vertex tangent in world space.
	vec3 bitangent_ws;	  // The vertex bitangent in world space.
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// Transforms the position, tangent, and bitangent into world space.
	// In case of the position we use 4x4 model matrix (as we did in the previous lectures).
	// In case of the tangent and bitangent we use 3x3 model matrix, i.e. use mat3(model) since we do not want to apply translation.
	out_data.position_ws = vec3(model * position);
	out_data.normal_ws = normalize(model_it * normal);
	out_data.tex_coord = tex_coord;
	out_data.tangent_ws = mat3(model) * tangent;
	out_data.bitangent_ws = mat3(model) * bitangent;

	gl_Position = projection * view * model * position;
}