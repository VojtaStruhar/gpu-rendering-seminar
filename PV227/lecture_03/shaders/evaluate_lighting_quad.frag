#version 450 core

#define LIGHTS_MAX_COUNT			150
#define LIGHTS_STORAGE				uniform		// Choose between 'uniform' and 'buffer'

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;
} in_data;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;	  // The projection matrix.
	mat4 projection_inv;  // The inverse of the projection matrix.
	mat4 view;			  // The view matrix
	mat4 view_inv;		  // The inverse of the view matrix.
	mat3 view_it;		  // The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;	  // The position of the eye in world space.
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

// The material data.
layout (std140, binding = 3) uniform PhongMaterialBuffer
{
    vec3 ambient;     // The ambient part of the material.
    vec3 diffuse;     // The diffuse part of the material.
    float alpha;      // The alpha (transparency) of the material.
    vec3 specular;    // The specular part of the material.
    float shininess;  // The shininess of the material.
} material;

// The texture with positoins.
layout (binding = 0) uniform sampler2D positions_tex;
// The texture with normals.
layout (binding = 1) uniform sampler2D normals_tex;
// The texture with albedo.
layout (binding = 2) uniform sampler2D albedo_tex;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 2: Evaluate the lighting.
	//	 Hint: Look into forward.frag to see what to use.
	final_color = vec4(1.0, 0.0, 0.0, 1.0);
}