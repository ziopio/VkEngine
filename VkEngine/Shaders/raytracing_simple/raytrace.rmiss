#version 460
#extension GL_EXT_ray_tracing : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;

layout(set = 2, binding = 0)uniform uniBlock {
	mat4 P_inverted;
	mat4 V_inverted;
	Light lights[10];
	int light_count;
} uniforms;

void main()
{
    hitValue = vec3(0.0, 0.1, 0.3);
}