#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
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
// The computed position.
layout (location = 0) out vec4 deferred_position;
// The computed normal.
layout (location = 1) out vec4 deferred_normal;
// The computed albedo.
layout (location = 2) out vec4 deferred_albedo;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 1: Compute deferred_position, deferred_normal, and deferred_albedo. Use diffuse color as albedo.
	//	 Hint: Look into forward.frag to see what to use.
	deferred_position = vec4(in_data.position_ws, 1.0f);
	deferred_normal = vec4(normalize(in_data.normal_ws), 0.0f);  // Zero is used because of homogenious coordinates - a good habit
	if(has_texture) {
		// TASK 1: Extract the albedo color from the texture.
		deferred_albedo = texture(material_diffuse_texture, in_data.tex_coord);
	} else {
		// TASK 1: Extract the albedo (i.e., diffuse) color from the material.
		deferred_albedo = vec4(material.diffuse, 1.0f);
	}
}