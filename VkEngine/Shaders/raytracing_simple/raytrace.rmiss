#version 460
#extension GL_EXT_ray_tracing : require

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 0, binding = 3) uniform samplerCube skybox;

const vec3 clear_color = {0.0, 0.1, 0.3};

void main()
{
    vec4 sky = texture(skybox, prd.rayDir);
    prd.hitValue.xyz = sky.xyz * (1.f - prd.hitValue.a)
                        + prd.hitValue.xyz * (prd.hitValue.a);
    prd.hitValue.a = 1.f;
}