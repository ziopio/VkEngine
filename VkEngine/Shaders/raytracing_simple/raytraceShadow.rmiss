#version 460
#extension GL_EXT_ray_tracing : require

#include "raycommon.glsl"

layout(location = 1) rayPayloadInEXT shadowPayload shadow;

void main()
{
  shadow.shadow_alpha += 0.f;
}
