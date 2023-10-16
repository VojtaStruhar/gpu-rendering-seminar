#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 position_vs;	  // The vertex position in view space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec3 normal_vs;		  // The vertex normal in view space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} in_data;

// The material data.
layout (std140, binding = 3) uniform PhongMaterialBuffer
{
    vec3 ambient;     // The ambient part of the material.
    vec3 diffuse;     // The diffuse part of the material.
    float alpha;      // The alpha (transparency) of the material.
    vec3 specular;    // The specular part of the material.
    float shininess;  // The shininess of the material.
} material;

// Uniform variable determining if we are using textures or not.
uniform bool has_texture;
// The texture that will be used (if available).
layout(binding = 0) uniform sampler2D material_diffuse_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The computed position in world space.
layout (location = 0) out vec4 deferred_position_ws;
// The computed position in view space.
layout (location = 1) out vec4 deferred_position_vs;
// The computed normal in world space.
layout (location = 2) out vec4 deferred_normal_ws;
// The computed normal in view space.
layout (location = 3) out vec4 deferred_normal_vs;
// The computed albedo.
layout (location = 4) out vec4 deferred_albedo;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// Computes deferred_position, deferred_normal, and deferred_albedo.
	deferred_position_ws = vec4(in_data.position_ws, 1.0);
	deferred_position_vs = vec4(in_data.position_vs, 1.0);
	deferred_normal_ws = vec4(normalize(in_data.normal_ws), 0.0f);
	deferred_normal_vs = vec4(normalize(in_data.normal_vs), 0.0);
	if(has_texture)
	{
		// Extracts the albedo color from the texture.
		deferred_albedo = texture(material_diffuse_texture, in_data.tex_coord);
	}else{
		// Extracts the albedo (i.e., diffuse) color from the material.
		deferred_albedo = vec4(material.diffuse, 1.0);
	}
}