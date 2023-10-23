#version 450 core

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
layout (std140, binding = 2) uniform PhongLightsBuffer
{
	vec3 global_ambient_color;				// The global ambient color.
	int lights_count;						// The number of lights in the buffer.
	PhongLight lights[8];                   // The array with actual lights.
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

// The texture with positoins in world space.
layout (binding = 0) uniform sampler2D positions_ws_tex;
// The texture with normals in world space.
layout (binding = 1) uniform sampler2D normals_ws_tex;
// The texture with albedo.
layout (binding = 2) uniform sampler2D albedo_tex;
// The texture with ssao.
layout (binding = 3) uniform sampler2D ssao_tex;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;
// The highlights.
layout (location = 1) out vec4 highlight_output;

float color_brightness(vec3 c) {
	return c.r * 0.299 + c.g * 0.587 + c.b * 0.114;
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// Extracts the information from the textures.
	vec3 position_ws = texture(positions_ws_tex, in_data.tex_coord).xyz;
	vec3 normal_ws = texture(normals_ws_tex, in_data.tex_coord).xyz;
	vec3 albedo = texture(albedo_tex, in_data.tex_coord).xyz;
	float ssao = texture(ssao_tex, in_data.tex_coord).r;

	// Computes the lighting.
	vec3 N = normalize(normal_ws);
	vec3 V = normalize(eye_position - position_ws);
	
	// Sets the starting coefficients.
	vec3 amb = global_ambient_color;
	vec3 dif = vec3(0.0);
	vec3 spe = vec3(0.0);

	// Processes all the lights.
	for (int i = 0; i < lights_count; i++)
	{
		vec3 L_not_normalized = lights[i].position.xyz - position_ws * lights[i].position.w;
		vec3 L = normalize(L_not_normalized);
		vec3 H = normalize(L + V);

		// Calculates the basic Phong factors.
		float Iamb = 1.0;
		float Idif = max(dot(N, L), 0.0);
		float Ispe = (Idif > 0.0) ? pow(max(dot(N, H), 0.0), material.shininess) : 0.0;

		// Calculates spot light factor.
		if (lights[i].spot_cos_cutoff != -1.0)
		{
			float spot_factor;
			float spot_cos_angle = dot(-L, lights[i].spot_direction);
			if (spot_cos_angle > lights[i].spot_cos_cutoff)
			{
				spot_factor = pow(spot_cos_angle, lights[i].spot_exponent);
			}
			else spot_factor = 0.0;

			Iamb *= 1.0;
			Idif *= spot_factor;
			Ispe *= spot_factor;
		}

		// Calculates attenuation point/spot lights.
		if (lights[i].position.w != 0.0)
		{
			float distance_from_light = length(L_not_normalized);
			float atten_factor =
				lights[i].atten_constant +
				lights[i].atten_linear * distance_from_light + 
				lights[i].atten_quadratic * distance_from_light * distance_from_light;
			atten_factor = 1.0 / atten_factor;

			Iamb *= atten_factor;
			Idif *= atten_factor;
			Ispe *= atten_factor;
		}

		// Applies the factors to light color.
		amb += Iamb * lights[i].ambient;
		dif += Idif * lights[i].diffuse;
		spe += Ispe * lights[i].specular;
	}

	// We use albedo values both for diffuse and ambient.
	vec3 mat_ambient = albedo;
	vec3 mat_diffuse = albedo;
	vec3 mat_specular = material.specular;

	// Computes the final light color.
	vec3 final_light = mat_ambient * amb + mat_diffuse * dif + material.specular * spe;

	// Outputs the final light color.
	// We apply the SSAO on all the light parts since the light ambient is set 0 in this lecture.
	final_color = vec4(final_light * ssao, material.alpha);

	// TASK 3: Write pixels with high brightness to a correct buffer.
	//   Hint: We are working with HDR buffers, treshold could be simply 1.0.
	//   Hint: Perceived brightness of pixel == r*0.299 + g*0.587 + b*0.114.

	// highlight_output = ...
	float b = color_brightness(final_color.rgb);
	if (b > 1.0f) {
		highlight_output = vec4(final_light * (b - 1.0f), 1.0f);
	} else {
		highlight_output = vec4(vec3(0.0), 1.0f);
	}
}
