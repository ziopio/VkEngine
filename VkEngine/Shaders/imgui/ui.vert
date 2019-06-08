#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
    int tex_ID;
} pc;

layout(location = 0) flat out int tex_index;
layout(location = 1) out vec2 UV;
layout(location = 2) out vec4 Color;

void main()
{
    Color = aColor;
    UV = aUV;
    tex_index = pc.tex_ID;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
