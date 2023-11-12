#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
    vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture to display.
layout (binding = 0) uniform sampler2D normal_tex;
layout (binding = 1) uniform sampler2D mirrored_tex;
layout (binding = 2) uniform sampler2D mask_tex;
layout (binding = 3) uniform sampler2D glass_tex;

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
    vec3 mask_color = texture(mask_tex, in_data.tex_coord).rgb;

    final_color = texture(normal_tex, in_data.tex_coord);

    /// Red channel is for the inside of the glass - for the mirroring part
    if (mask_color.r > 0.5)
    {
        final_color = texture(mirrored_tex, in_data.tex_coord);
    }

    /// Green channel means the outside view - half transparent glass object / texture
    if (mask_color.g > 0.5)
    {
        final_color = vec4(mix(final_color.rgb, texture(glass_tex, in_data.tex_coord).rgb, 0.5), final_color.a);
    }

}