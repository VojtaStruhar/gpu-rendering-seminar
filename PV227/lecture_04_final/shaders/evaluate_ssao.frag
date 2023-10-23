#version 450 core

// The number of the samples.
#define NUMBER_OF_SSAO_SAMPLES 64

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

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

// The random positions of the SSAO kernel.
layout (std140, binding = 1) uniform KernelSamples
{
	vec4 kernel_samples[NUMBER_OF_SSAO_SAMPLES];
};

// The texture with positoins in view space.
layout (binding = 0) uniform sampler2D positions_vs_tex;
// The texture with normals in view space.
layout (binding = 1) uniform sampler2D normals_vs_tex;
// The texture with random tangents in view space.
layout (binding = 2) uniform sampler2D random_tangent_vs_tex;

// The radius of the SSAO hemisphere which is sampled.
uniform float ssao_radius;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The computed occlusion.
layout (location = 0) out float final_occlusion;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// Extracts positions and normals in view space.
	vec4 position_vs = texture(positions_vs_tex, in_data.tex_coord);
	vec3 normal_vs = normalize(texture(normals_vs_tex, in_data.tex_coord).xyz);

	// Computes tangent and bitangent (in view space), randomly rotated around z-axis.
	// gl_FragCoord.xy contains position of the fragment in window space, i.e., its values are from (viewport.x, viewport.y) to (viewport.x + viewport.width, viewport.y + viewport.height).
	// We use this fragment position as a texture coordinate and sample random tangent texture.
	// We get a tangent that is not necessarly perpendicular to the normal (at lest not yet).
	vec3 tangent_vs = texture(random_tangent_vs_tex, gl_FragCoord.xy / vec2(textureSize(random_tangent_vs_tex, 0))).xyz;
	// Computes the bitangent so that it is perpendicular to both the tangent and the normal.
	vec3 bitangent_vs = normalize(cross(normal_vs, tangent_vs));
	// Recomputes the tangent so that it is perpendicular to both the normal and the bitangent.
	tangent_vs = normalize(cross(bitangent_vs, normal_vs));
	// Creates a 3x3 matrix from the tangent, bitangent, and normal.
	// TBN * V.xyz == tangent * V.x + bitangent * V.y + normal * V.z
	mat3 TBN = mat3(tangent_vs, bitangent_vs, normal_vs);

	// Computes the number of samples that are (not) occluded.
	int occluded_samples = 0, not_occluded_samples = 0;
	for (int i = 0; i < NUMBER_OF_SSAO_SAMPLES; i++)
	{
		// Computes the position of the sample in view space
		vec3 sample_offset_vs = TBN * kernel_samples[i].xyz * ssao_radius;
		vec3 sample_position_vs = position_vs.xyz + sample_offset_vs;
		// Transforms the position into the clip space ...
		vec4 sample_position_cs = projection * vec4(sample_position_vs, 1.0);
		// ... and then into normalized device coordinates, ...
		vec3 sample_position_nds = sample_position_cs.xyz / sample_position_cs.w;
		// ... so that we can obtain the position of the closest object at that sample (in view space)
		vec4 closest_object_vs = textureLod(positions_vs_tex, sample_position_nds.xy * 0.5 + 0.5, 0);

		// Compares the distance.
		if (closest_object_vs.w == 0.0)
		{
			// There is no occluder (object_at_sample.w == 0.0 means the background).
			
			// TASK 1: Evaluate the occlusion for case a), add one to occluded_samples if this sample is occluded, 
			//         add one to not_occluded_samples if the sample is not occluded.
			not_occluded_samples++;
		}
		else
		{
			// We compare the distances from the camera of the occludee (the sample) and the occluder (the object).
			
			// TASK 1: Evaluate the occlusion for cases b),c):
			//	 - Compute the distances of the sample (its position is in sample_position_vs) and the possible occluder (its position is in closest_object_vs).
			//		Eye position in view space is (0,0,0).
			//	    Hint: Think how the position in *view* space is related to the distance from the camera.
			//	 - Compare the distances and evaluate the occlusion. Add one to occluded_samples if this sample is occluded, add one to not_occluded_samples if the sample is not occluded.
			
			// TASK 3: Evaluate the occlusion for case d):
			//	 - Use ssao_radius to choose between c) and d) - if the occluder is closer to the viewer by more than ssao_radius, it is d). 
			//      Hint: Remember, we ignore such samples.
			float sample_distance = -sample_position_vs.z;	    // or length(sample_position_vs)
			float occluder_distance = -closest_object_vs.z;     // or length(closest_object_vs.xyz)
			if ((sample_distance - occluder_distance) >= ssao_radius)
			{
				// This is d).
				// There is an occluder, but the occluder is a lot closer than the sample. 
				// In this example, we say all objects have the same thickness which is the radius of the hemisphere.
				// The occluder object may or may not be thick enough to affect the ambient occlusion, so we ignore this sample. 
			}
			else 
			{
				// This is b,c)
				// There is an occluder, and it is not too close. We compare the distances and decide whether the sample is occluded or not.
				if (sample_distance > occluder_distance)
					occluded_samples++;
				else
					not_occluded_samples++;
			}
		}
	}

	// Computes the final occlusion.
	if (position_vs.w == 0.0)
		// We processed a pixel of the background, we set its final occlusion to zero.
		final_occlusion = 0.0;		
	else if ((occluded_samples + not_occluded_samples) == 0)
		// We found no valid sample to test, so we say there is no occlusion.
		final_occlusion = 1.0;		
	else
		// In all other cases we compute the ratio between occluded and non occluded samples to estimate the overall occlution.
		final_occlusion = float(not_occluded_samples) / float(occluded_samples + not_occluded_samples);

}