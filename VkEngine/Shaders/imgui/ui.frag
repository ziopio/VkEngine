#version 450 core
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in flat int tex_index;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec4 Color;

layout(location = 0) out vec4 fColor;

layout(set=0, binding=0) uniform sampler2D sTexture[32];

void main()
{
    fColor = Color * texture(sTexture[tex_index], UV);
}
