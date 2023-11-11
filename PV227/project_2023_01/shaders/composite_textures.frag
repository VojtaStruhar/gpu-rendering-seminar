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
    // We applie the transformation to the color from the texture using the uniform matrix.
    final_color = texture(normal_tex, in_data.tex_coord).rgba;
}