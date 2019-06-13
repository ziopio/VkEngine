#version 450 core
#extension GL_ARB_separate_shader_objects : enable
//input
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
//uniforms
layout(push_constant) uniform uPushConstant {
    vec2 uScale;
    vec2 uTranslate;
    int tex_ID;
} pc;
//outputs
layout(location = 0) out vec2 UV;
layout(location = 1) out vec4 Color;
layout(location = 2) flat out int tex_index;

void main()
{
    Color = aColor;
    UV = aUV;
    tex_index = pc.tex_ID;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
