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
	float tm1 = 1 - t;

	return v0 * tm1 *tm1*tm1 + 3*v1*tm1*tm1*t + 3*v2*tm1*t*t + v3 * t*t*t;
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
	vec3 r0 = bezier4(v0, v1, v2, v3, t.x);
	vec3 r1 = bezier4(v4, v5, v6, v7, t.x);
	vec3 r2 = bezier4(v8, v9, v10, v11, t.x);
	vec3 r3 = bezier4(v12, v13, v14, v15, t.x);

	return bezier4(r0, r1, r2, r3, t.y);
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
	out_data.position_ws = bezier4x4(
		in_data[0].position_ws, in_data[1].position_ws, in_data[2].position_ws, in_data[3].position_ws,
		in_data[4].position_ws, in_data[5].position_ws, in_data[6].position_ws, in_data[7].position_ws,
		in_data[8].position_ws, in_data[9].position_ws, in_data[10].position_ws, in_data[11].position_ws,
		in_data[12].position_ws, in_data[13].position_ws, in_data[14].position_ws, in_data[15].position_ws,
		gl_TessCoord.xy);

	// TASK 2: Compute the texture coordinates, use the patch coordinates as texture coordinates.
	//         Store the result into out_data.tex_coord.
	out_data.tex_coord = gl_TessCoord.xy;

	// TASK 3: Evaluate the tangent and bitangent of the vertex in the Bezier patch.
	//         Store the result into out_data.tangent_ws/bitangent_ws.
	//   Hint: Use the same functions as for the position.
	out_data.tangent_ws = bezier4x4(
		in_data[0].tangent_ws, in_data[1].tangent_ws, in_data[2].tangent_ws, in_data[3].tangent_ws,
		in_data[4].tangent_ws, in_data[5].tangent_ws, in_data[6].tangent_ws, in_data[7].tangent_ws,
		in_data[8].tangent_ws, in_data[9].tangent_ws, in_data[10].tangent_ws, in_data[11].tangent_ws,
		in_data[12].tangent_ws, in_data[13].tangent_ws, in_data[14].tangent_ws, in_data[15].tangent_ws,
		gl_TessCoord.xy);
	out_data.tangent_ws = normalize(out_data.tangent_ws);


	out_data.bitangent_ws = bezier4x4(
	in_data[0].bitangent_ws, in_data[1].bitangent_ws, in_data[2].bitangent_ws, in_data[3].bitangent_ws,
	in_data[4].bitangent_ws, in_data[5].bitangent_ws, in_data[6].bitangent_ws, in_data[7].bitangent_ws,
	in_data[8].bitangent_ws, in_data[9].bitangent_ws, in_data[10].bitangent_ws, in_data[11].bitangent_ws,
	in_data[12].bitangent_ws, in_data[13].bitangent_ws, in_data[14].bitangent_ws, in_data[15].bitangent_ws,
	gl_TessCoord.xy);
	out_data.bitangent_ws = normalize(out_data.bitangent_ws);

	// TASK 3: Compute the normal as the cross product of the tangent and bitangent, and normalize it.
	//         Store the result into out_data.normal_ws.
	out_data.normal_ws = normalize(cross(out_data.tangent_ws, out_data.bitangent_ws));


	if (use_displacement_mapping)
	{
		// TASK 5: Displace the vertex position.
		//   Hint: Functions: textureLod
		//         texture is stored in 'height_tex'
		//         constant 'displacement_factor' defines a requested multiplier for the height of the displacement
		float height = textureLod(height_tex, out_data.tex_coord, 0).r;
		out_data.position_ws += out_data.normal_ws * height * displacement_factor;
	}

	// Computes gl_Position.
	gl_Position = projection * view * vec4(out_data.position_ws, 1.0);
}