#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
	vec3 tangent_ws;	  // The vertex tangent in world space.
	vec3 bitangent_ws;	  // The vertex bitangent in world space.
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
layout (std140, binding = 2) uniform PhongLightsBuffer
{
	vec3 global_ambient_color;		 // The global ambient color.
	int lights_count;				 // The number of lights in the buffer.
	PhongLight lights[8];			 // The array with actual lights.
};

// The data of the material.
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

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
	// Computes the lighting.
	vec3 N = normalize(in_data.normal_ws);
	vec3 V = normalize(eye_position - in_data.position_ws);
	
	// Sets the starting coefficients - we omit the global ambient light in this lecture.
	vec3 amb = vec3(0.0);
	vec3 dif = vec3(0.0);
	vec3 spe = vec3(0.0);

	// Processes all the lights.
	for (int i = 0; i < lights_count; i++)
	{
		vec3 L_not_normalized = lights[i].position.xyz - in_data.position_ws * lights[i].position.w;
		vec3 L = normalize(L_not_normalized);
		vec3 H = normalize(L + V);

		// Computes the ambient part. Ambient light is a bit tricky, because it does not exist in PBR.
		// We use material's diffuse color and also very small ambient light, so that we can see
		// at least some of the shape of the object in the dark parts, do not make the light too bright.
		amb += lights[i].ambient * material_diffuse;

		// TASK 1: Compute the diffuse part.
		//	 Hint: Vectors N, V, H, and L are prepared, light's intensity and color is in lights[i].diffuse, 'Color_diff' is in material_diffuse.
		dif += lights[i].diffuse * material_diffuse * max(0.0, dot(N, L));

		// TASK 1: Compute the specular part.
		//	 Hint: Vectors N, V, H, and L are prepared. 'Color_spe' is in material_specular, 'shin' is in material_shininess.
		spe += lights[i].specular * material_specular * max(dot(N, L), 0.0) * (material_shininess + 8.0) / 8.0 * pow(max(dot(N, H), 0.0), material_shininess);

		// In this lecture we omit the spot lights for simplicity.
		// In this lecture we do not apply any attenuation for simplicity.
	}

	// Adds all three parts of the color together.
	vec3 final_light = amb + dif + spe;
	final_color = vec4(final_light, 1.0);
}