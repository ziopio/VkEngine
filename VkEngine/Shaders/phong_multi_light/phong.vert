#version 460
#extension GL_ARB_separate_shader_objects:enable

#include "..\commons.glsl"

layout(location=0)in vec3 inPosition;
layout(location=1)in vec3 inNormal;
layout(location=2)in vec3 inColor;
layout(location=3)in vec2 inTexCoord;

layout(location=0)out vec3 fragColor;
layout(location=1)out vec2 fragTexCoord;
layout(location=2)flat out int outTextureIndex;
layout(location=3)out vec3 P;
layout(location=4)out vec3 N;
layout(location=5)out vec3 E;

layout(set=1,binding=0)uniform uniBlock{
	mat4 P;
	mat4 V;
	Light global_light;
	Light lights[10];
	int light_count;
}uniforms;
layout(push_constant)uniform PushConsts
{
	mat4 M;// 64 bytes (vec4 *4)
	int textureIndex;// 4 bytes
}pushConsts;

void main()
{
	vec4 vertexWorldPos = pushConsts.M * vec4(inPosition,1);
	gl_Position = uniforms.P * uniforms.V * vertexWorldPos;
	fragColor=inColor;
	outTextureIndex=pushConsts.textureIndex;
	fragTexCoord=inTexCoord;
	
	P = vertexWorldPos.xyz;		
	E = vertexWorldPos.xyz - (inverse(uniforms.V) * vec4(0,0,0,1)).xyz;
	N = transpose(inverse(mat3(pushConsts.M))) * inNormal;
}