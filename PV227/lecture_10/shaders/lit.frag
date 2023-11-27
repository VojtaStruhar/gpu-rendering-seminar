#version 450 core

layout (early_fragment_tests) in;

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

// Random numbers for random sampling directions
layout (std140, binding = 4) uniform RandomFloats
{
	vec4 random_values[1024];
};

// The cubemap texture with skybox.
layout (binding = 0) uniform samplerCube cubemap_tex;
// The texture with mask.
layout (binding = 1) uniform sampler2D mask_tex;

// Data of the material.
uniform vec3 material_diffuse;
uniform vec3 material_fresnel;
uniform float material_roughness;

// Parameters of the computation.
uniform int diffuse_sample_count;
uniform int specular_sample_count;
uniform bool mask_roughness;
uniform bool use_shiny_layer;
uniform bool use_shiny_layer_mask;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Local Methods
// ----------------------------------------------------------------------------

vec3 FresnelSchlick(in vec3 f0, in vec3 V, in vec3 H)
{
	float VdotH = clamp(dot(V, H), 0.001, 1.0);
	return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5.0);
}

float GeometricAttenuation(in vec3 N, in vec3 V, in vec3 L, in vec3 H)
{
	float NdotH = clamp(dot(N, H), 0.001, 1.0);
	float NdotV = clamp(dot(N, V), 0.001, 1.0);
	float VdotH = clamp(dot(V, H), 0.001, 1.0);
	float NdotL = clamp(dot(N, L), 0.001, 1.0);
	return min(1.0, min(2.0 * NdotH * NdotV / VdotH, 2.0 * NdotH * NdotL / VdotH));
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float mask_value = texture(mask_tex, in_data.tex_coord).r;

	// Basic variables.
	vec3 N = normalize(in_data.normal_ws);
	if (!gl_FrontFacing) {
		N = -N;
	}
	vec3 T = normalize(in_data.tangent_ws);
	vec3 B = normalize(in_data.bitangent_ws);
	vec3 V = normalize(eye_position - in_data.position_ws);
	
	float NdotV = clamp(dot(N, V), 0.001, 1.0);

	// TASK 1: Implement diffuse lighting
	//		Hints:
	//			Don't forget to clamp all dot products. Also make them greater than 0.001 to avoid divisions by zero.
	//			Don't forget to normalize L vector.
	//			The diffuse color (C_diff) of the material is in 'material_diffuse'.
	//			Environment is in 'cubemap_tex'.
	//			Number of samples is in 'diffuse_sample_count'.
	//		Experiments:
	//			Try increasing the number of samples (from GUI). Also when sampling 'cubemap_tex', try sampling higher mipmap levels (use textureLod). 
	//			In both experiments, notice the quality of the image and rendering times.
	vec3 dif = vec3(0.0);
	for (int i = 0; i < diffuse_sample_count; i++)
	{
		vec2 R = random_values[i].xy;

		//dif += ...;
	}

	// TASK 2: Implement specular lighting
	//	Hints:
	//		Don't forget to clamp all dot products. Also, make them greater than 0.001 to avoid divisions by zero.
	//		Make sure all values are non-negative before computing sqrt, to increase the robustness.
	//		Don't forget to normalize H vector.
	//		Number of samples is in 'specular_sample_count'.
	//		F0 is in 'material_fresnel' (again, it is a vector).
	//		Use the functions FresnelSchlick and GeometricAttenuation that computes Fresnel and geom. atten. (already implemented)
	//	Experiments:
	//		Try increasing the number of samples (from GUI) and sampling higher mipmap levels (use textureLod).
	//		Also try changing roughness according to the mask texture (this texture is already sampled, the value is in 'mask_value' variable). 
	//		Think about whether the computed color fits into its surrounding.
	vec3 spe = vec3(0.0);
	for (int i = 0; i < specular_sample_count; i++)
	{
		vec2 R = random_values[i].zw;

		//spe += ...;
	}

	// TASK 3: Implement a shiny layer. Feel free to experiment. Try the semitransparent layer.
	//	Hints:  Use function reflect to get the direction of perfect reflection.
	//			Use FresnelSchlick to estimate the amount of reflected light -- non-metals have their F0 very low, 0.04 is a good choice.
	//		    Modify the already computed dif and spe values using the computed Fresnel-Schlick value as coeficient. 
	//			   -- i.e., suppress the already computed values in dif and spe if there is a perfect reflection (keep the rest unmodified)
	//			   -- in areas with perfect reflection add the intensity obtained from the cubemap
	if (use_shiny_layer)
	{
		//dif = ...;
		//spe = ...;
	}

	// Add all three parts of the color together.
	vec3 final_light = dif + spe;
	final_color = vec4(final_light, 1.0);
}