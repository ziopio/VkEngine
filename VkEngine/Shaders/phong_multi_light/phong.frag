#version 460
#extension GL_ARB_separate_shader_objects:enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "..\commons.glsl"

layout(location=0)in vec3 fragColor;
layout(location=1)in vec2 fragTexCoord;
layout(location=2)flat in int inTextureIndex;
layout(location=3)in vec3 fragPos;
layout(location=4)in vec3 normal;
layout(location=5)in vec3 eyeDir;

layout(set=0,binding=0)uniform sampler2D texSamplers[];
layout(set=1,binding=0)uniform uniBlock{
	mat4 P;
	mat4 V;
	Light global_light;
	Light lights[10];
	int light_count;
}uniforms;

layout(location=0)out vec4 outColor;

void main(){
	vec4 texel = texture(texSamplers[inTextureIndex],fragTexCoord);
	
	vec3 color = {0,0,0};
	vec3 ambient = texel.xyz * 0.01;
	vec3 N = normalize(normal);
	vec3 E = normalize(eyeDir);

	vec3 sunDir = normalize(uniforms.global_light.position.xyz);
	vec3 globalDiffuse = computeDiffuse(texel.xyz, uniforms.global_light, sunDir, N);
	vec3 globalSpecular = computeSpecular(uniforms.global_light, E, sunDir, N);
	color += globalDiffuse + globalSpecular;

	for(int i=0;i<uniforms.light_count;i++){
		// Distance to the light
		vec3 lightVector = uniforms.lights[i].position.xyz - fragPos;
		vec3 L = normalize(lightVector);
		float lightDst = length(lightVector);
		float attenuation = LIGTH_ATTENUATION(lightDst);
		vec3 diffuse = computeDiffuse(texel.xyz, uniforms.lights[i], L, N);
		vec3 specular = computeSpecular(uniforms.lights[i], E, L, N);
		color += (diffuse * attenuation + specular * attenuation);
	}
	outColor = vec4 (ambient + color / (uniforms.light_count + 1) ,texel.a); // alpha correction
}