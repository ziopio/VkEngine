#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in int textureIndex;


layout(set = 0, binding = 0) uniform sampler2D texSamplers[1024];


layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSamplers[textureIndex], fragTexCoord); // + fragColor;
}