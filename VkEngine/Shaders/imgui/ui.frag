#version 450 core
#extension GL_ARB_separate_shader_objects : enable
//input
layout(location = 0) in vec2 UV;
layout(location = 1) in vec4 Color;
layout(location = 2) in flat int tex_index;
//uniforms
layout(set=0, binding=0) uniform sampler2D sTexture[32];
//outputs
layout(location = 0) out vec4 fColor;

void main()
{
    fColor = Color * texture(sTexture[tex_index], UV);
}
