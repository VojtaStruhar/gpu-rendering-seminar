#version 450 core

#extension GL_ARB_shader_viewport_layer_array : require

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
layout (location = 0) in vec4 position;  // The vertex position.
layout (location = 1) in vec3 normal;	 // The vertex normal.
layout (location = 2) in vec2 tex_coord; // The vertex texture coordinates.

// The data of a single camera.
struct SingleCameraData
{
	mat4 projection;		// The projection matrix.
	mat4 projection_inv;	// The inverse of the projection matrix.
	mat4 view;				// The view matrix
	mat4 view_inv;			// The inverse of the view matrix.
	mat3 view_it;			// The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// The position of the eye in world space.
};

// The uniform buffer with all camera data.
layout (std140, binding = 0) uniform CubeCameraData
{
	SingleCameraData cameras[6];
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
	// Computes the data of the vertex.
	out_data.tex_coord = tex_coord;
	out_data.position_ws = vec3(model * position);
	out_data.normal_ws = normalize(model_it * normal);

	// TASK 4: Compute gl_Position using matrices from a correct camera and set a different gl_Layer for each instance.
	//  Hints: gl_InstanceID contains the index of the current primitive in an instanced draw command.

	// Sets the layer from gl_InstanceID.
	gl_Layer = gl_InstanceID;

	// Computes the gl_Position using the proper camera.
	gl_Position = cameras[gl_InstanceID].projection * cameras[gl_InstanceID].view * model * position;
}