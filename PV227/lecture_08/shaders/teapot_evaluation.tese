#version 450 core

// The input primitive specification.
layout(quads, fractional_odd_spacing, ccw) in;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 tangent_ws;	  // The vertex tangent in world space.
	vec3 bitangent_ws;	  // The vertex bitangent in world space
} in_data[];

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

// The height texture and a scaling factor applied to it.
layout (binding = 2) uniform sampler2D height_tex;
uniform float displacement_factor;

// The flag determining whether to displacement it or not.
uniform bool use_displacement_mapping;

// ----------------------------------------------------------------------------
// Custom Methods
// ----------------------------------------------------------------------------

// Evaluates cubic bezier spline.
vec3 bezier4(in vec3 v0, in vec3 v1, in vec3 v2, in vec3 v3, in float t)
{
	// TASK 1: Evaluate cubic bezier spline.
	//   Hint: Precompute (1-t) and store it into a variable for optimization and better readability.
	return v1;
}

// Evaluates cubic bezier patch.
vec3 bezier4x4(
	in vec3 v0,  in vec3 v1,  in vec3 v2,  in vec3 v3,
	in vec3 v4,  in vec3 v5,  in vec3 v6,  in vec3 v7,
	in vec3 v8,  in vec3 v9,  in vec3 v10, in vec3 v11,
	in vec3 v12, in vec3 v13, in vec3 v14, in vec3 v15, in vec2 t)
{
	// TASK 1: Evaluate cubic bezier patch.
	//   Hint: Use bezier4 function defined above.
	//         Use t.x and t.y.
	return v1;
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 1: Evaluate the position of the vertex in the Bezier patch.
	//		   Store the result into out_data.position_ws
	//   Hint: First implement functions bezier4 and bezier4x4 above. 
	//         Patch coordinates are in gl_TessCoord.xy.
	out_data.position_ws = vec3(0.0);

	// TASK 2: Compute the texture coordinates, use the patch coordinates as texture coordinates.
	//         Store the result into out_data.tex_coord.

	// TASK 3: Evaluate the tangent and bitangent of the vertex in the Bezier patch.
	//         Store the result into out_data.tangent_ws/bitangent_ws.
	//   Hint: Use the same functions as for the position.

	// TASK 3: Compute the normal as the cross product of the tangent and bitangent, and normalize it.
	//         Store the result into out_data.normal_ws.

	if (use_displacement_mapping)
	{
		// TASK 5: Displace the vertex position.
		//   Hint: Functions: textureLod
		//         texture is stored in 'height_tex'
		//         constant 'displacement_factor' defines a requested multiplier for the height of the displacement
	}

	// Computes gl_Position.
	gl_Position = projection * view * vec4(out_data.position_ws, 1.0);
}