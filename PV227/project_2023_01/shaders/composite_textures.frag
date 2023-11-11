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

    if (mask_color.r > 0.5)
    {
        final_color = texture(mirrored_tex, in_data.tex_coord);
    }
    else
    {
        final_color = texture(normal_tex, in_data.tex_coord);
    }

}