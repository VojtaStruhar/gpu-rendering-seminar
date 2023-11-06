#version 450 core

#extension GL_ARB_shader_viewport_layer_array : require

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
// Texture coordinates for the full screen quad.
const vec2 tex_coords[3] = vec2[3] (
	vec2(0.0, 0.0),
	vec2(2.0, 0.0),
	vec2(0.0, 2.0)
);

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

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	vec3 tex_coord;		  // The vertex texture coordinates.
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 4: Update the algorithm from skybox.vert to utilize matrices from a correct camera and set gl_Layer.
	//  Hints: gl_InstanceID contains the index of the current primitive in an instanced draw command.
}