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
uniform vec3 material_fresnel;
uniform float material_shininess_tangent;
uniform float material_shininess_bitangent;

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
	float VdotH = dot(V, H);
	return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5.0);
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// Computes the lighting.
	vec3 N = normalize(in_data.normal_ws);
	vec3 T = normalize(in_data.tangent_ws);
	vec3 B = normalize(in_data.bitangent_ws);
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
		// We use the average of diffuse and fresnel and also very small ambient light, so that we can see
		// at least some of the shape of the object in the dark parts, do not make the light too bright.
		amb += (material_diffuse + material_fresnel)*0.5 * lights[i].ambient;

		// Task 4: Compute the diffuse part.
		//	 Hint: Vectors N, T, B, V, H, and L are prepared, light's intensity and color is in lights[i].diffuse, 'Color_diff' is in material_diffuse, 'F_0' is in material_fresnel.

		// More precise energy conservation.
		dif += material_diffuse * lights[i].diffuse * max(0.0, dot(N, L)) *
			28.0/23.0 * (1.0 - material_fresnel) * 
			(1.0 - pow(1.0 - max(0.0, dot(N, L)) / 2.0, 5.0)) *
			(1.0 - pow(1.0 - max(0.0, dot(N, V)) / 2.0, 5.0));
		// Simple version (as in standard Blinn-Phong)
		// dif += material_diffuse * lights[i].diffuse * max(0.0, dot(N, L));

		// Task 4: Compute the specular part.
		//		Hint: Copy-paste function FresnelSchlick from CookTorrance_fragment.glsl
		//		Hint: Vectors N, T, B, V, H, and L are prepared, light's intensity and color is in lights[i].diffuse,
		//			F_0 is in material_fresnel, shin_T and shin_B are in material_shininess_tangent and material_shininess_bitangent

		// First part - the square root and division by 8.
		float spe_first_part = sqrt((material_shininess_tangent + 1.0) * (material_shininess_bitangent + 1.0)) / 8.0;
		// Second part - exponent's numerator.
		float spe_exponent_numerator = material_shininess_tangent * dot(T,H) * dot(T, H) + material_shininess_bitangent * dot(B, H) * dot(B, H);
		// Second part - exponent's denominator.
		float spe_exponent_denominator = 1.0 - dot(H, N)*dot(H, N);
		// Second part - fraction's denominator.
		float spe_fraction_denominator = dot(V,H) * max(dot(N, L), dot(N, V));
		// Second part - all together.
		float spe_second_part = pow(max(0.0, dot(N, H)), spe_exponent_numerator / spe_exponent_denominator) / spe_fraction_denominator;
		// All toghether.
		spe += lights[i].specular * FresnelSchlick(material_fresnel, V, H) * max(0.0, dot(N, L)) * spe_first_part * spe_second_part;

		// In this lecture we omit the spot lights for simplicity.
		if (lights[i].spot_cos_cutoff != -1.0)
		{
			// nothing to implement here
		}

		//  In this lecture we do not apply any attenuation for simplicity.
		if (lights[i].position.w != 0.0)
		{
			// nothing to implement here
		}
	}

	// Adds all three parts of the color together.
	vec3 final_light = amb + dif + spe;
	final_color = vec4(final_light, 1.0);
}