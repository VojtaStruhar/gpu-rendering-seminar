#version 450 core

#define LIGHTS_MAX_COUNT			2000
#define LIGHTS_STORAGE				buffer		// Choose between 'uniform' and 'buffer'

// ----------------------------------------------------------------------------
// Variables
// ----------------------------------------------------------------------------
// The considered light range.
const float light_range = 8.0f;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// The position of the vertex of the unit sphere we render.
layout (location = 0) in vec4 position;

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

// The structure holding the information about a single Phong light.
struct PhongLight
{
	vec4 position;                   // The position of the light. Note that position.w should be one for point lights and spot lights, and zero for directional lights.
	vec3 ambient;                    // The ambient part of the color of the light.
	vec3 diffuse;                    // The diffuse part of the color of the light.
	vec3 specular;                   // The specular part of the color of the light. 
	vec3 spot_direction;             // The direction of the spot light, irrelevant for point lights and directional lights.
	float spot_exponent;             // The spot exponent of the spot light, irrelevant for point lights and directional lights.
	float spot_cos_cutoff;           // The cosine of the spot light's cutoff angle, -1 point lights, irrelevant for directional lights.
	float atten_constant;            // The constant attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 1.
	float atten_linear;              // The linear attenuation of spot lights and point lights, irrelevant for directional lights.  For no attenuation, set this to 0.
	float atten_quadratic;           // The quadratic attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 0.
};

// The UBO with light data.
layout (std140, binding = 2) LIGHTS_STORAGE PhongLightsBuffer
{
	vec3 global_ambient_color;				// The global ambient color.
	int lights_count;						// The number of lights in the buffer.
	PhongLight lights[LIGHTS_MAX_COUNT];    // The array with actual lights.
};

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	noperspective vec2 tex_coord;		// The texture coordinate for sampling G-buffer textures.
	flat int light_idx;					// The index of the light that is being processed.
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 3: Compute the correct world space position for each sphere vertex -> enlarge the unit sphere by light_range and move it to the light potision.
	//         Transform this world space position into the clip space (i.e. transform it by the view and pojection matrices).
	//         Store the position into pos_cs.
    //   Hint: position.xyz is the position of the vertex of the unit sphere we render
	//         lights[...].position.xyz is the position of the light in world space
	//         light_range is the range into which the light shines.
	//		   gl_InstanceID is the index the current instance, which is also the index of the light that is processed
	
	// Computes the position in world space.
	vec3 pos_ws = position.xyz * light_range + lights[gl_InstanceID].position.xyz;
	// Computes the position in clip space.
	vec4 pos_cs = projection * view * vec4(pos_ws, 1.0);

	// Sets the output variables.
	out_data.light_idx = gl_InstanceID;
	out_data.tex_coord = pos_cs.xy / pos_cs.w * 0.5 + 0.5;
	gl_Position = pos_cs;
}