#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#include "..\commons.glsl"
#include "raycommon.glsl"

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1, scalar) buffer Vertices { Vertex3D vertices[]; } vertexBuffers[];
layout(set = 0, binding = 2) buffer Indices { uint indices[]; } indexBuffers[];
layout(set = 0, binding = 3) uniform sampler2D texSamplers[];

layout(set = 1, binding = 1, scalar) buffer SceneDesc {ObjDesc obj[];} sceneObjects;

layout(set = 2, binding = 0)uniform uniBlock {
	mat4 P_inverted;
	mat4 V_inverted;
  Light global_light;
	Light lights[10];
	int light_count;
} uniforms;

void castShadowRay(vec3 lightDirection, float lightDistance);

void main()
{
  ObjDesc object = sceneObjects.obj[gl_InstanceID];
  // Object of this instance
  uint mesh_id = object.meshId;

  //Indices of the triangle
  ivec3 indices = ivec3(indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 0], 
                        indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 1], 
                        indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 2]);

  //Vertices of the triangle
  Vertex3D v0 = vertexBuffers[nonuniformEXT(mesh_id)].vertices[indices.x];
  Vertex3D v1 = vertexBuffers[nonuniformEXT(mesh_id)].vertices[indices.y];
  Vertex3D v2 = vertexBuffers[nonuniformEXT(mesh_id)].vertices[indices.z];

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the normal at hit position
  vec3 normal = v0.nrm.xyz * barycentrics.x + 
                v1.nrm.xyz * barycentrics.y + 
                v2.nrm.xyz * barycentrics.z;

  normal = normalize(vec3( object.transform * vec4(normal, 0.0)));
  
    // Computing the coordinates of the hit position
  vec3 worldPos = v0.pos * barycentrics.x + 
                  v1.pos * barycentrics.y + 
                  v2.pos * barycentrics.z;
  // Transforming the position to world space
  worldPos = vec3(object.transform * vec4(worldPos, 1.0));
  
  // Gather UV coordinates from the vertices and the baricentric coordinates
  vec2 UV = v0.texCoord * barycentrics.x + 
            v1.texCoord * barycentrics.y + 
            v2.texCoord * barycentrics.z;
  vec4 albedo = texture(texSamplers[nonuniformEXT(object.textureId)],UV);
  vec3 ambient = albedo.xyz * 0.01;
  float shadow_attenuation = 0.3;
  vec3 color = {0,0,0};
  //Global direcitonal light
  vec3 L = normalize(uniforms.global_light.position.xyz);
  float sun_distance = 1000000.0;
  if(object.reflective==0) color = computeDiffuse(albedo.xyz, uniforms.global_light, L, normal);
  if( facingLight(normal, L) )
  {
      castShadowRay(L, sun_distance);
      if(isShadowed)
        color *= shadow_attenuation;
      else
        color += computeSpecular(uniforms.global_light, gl_WorldRayDirectionEXT, L, normal);
  }
  // Point lights
  for (int i = 0; i < uniforms.light_count ;i++){
    vec3 lightVector = uniforms.lights[i].position.xyz - worldPos;
    vec3 lDir = normalize(lightVector);
    float lightDistance = length(lightVector); 
    vec3 c= {0,0,0};
    if(object.reflective==0) c = computeDiffuse(albedo.xyz, uniforms.lights[i], lDir, normal);
    if( facingLight(normal, lDir) )
    {
      castShadowRay(lDir, lightDistance);
      if(isShadowed)
        c *= shadow_attenuation;
      else
        c += computeSpecular(uniforms.lights[i], gl_WorldRayDirectionEXT, lDir, normal);
    }
    c *= LIGTH_ATTENUATION(lightDistance);
    color += c;
  }


  // BLENDING
  prd.hitValue.xyz =  
  vec3(ambient + color / (uniforms.light_count + 1)) * (1.f - prd.hitValue.a) 
  + prd.hitValue.xyz * (prd.hitValue.a);
  

  // Reflection
  if(object.reflective > 0)
  {
    vec3 origin = worldPos;
    vec3 rayDir = reflect(gl_WorldRayDirectionEXT, normal);
    prd.attenuation *= 1.0f; //mat.specular;
    prd.done      = 0;
    prd.rayOrigin = origin;
    prd.rayDir    = rayDir;

  } else 
  {
    prd.hitValue.a += albedo.a;
  }
}

void castShadowRay(vec3 rayDir, float lightDistance){
      float tMin = 0.001;
      float tMax = length(lightDistance);
      vec3  origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
      uint  flags = 
      //gl_RayFlagsTerminateOnFirstHitEXT | // The first hit is always good.
      //gl_RayFlagsOpaqueEXT | // Will not call the any hit shader, so all objects will be opaque.
      gl_RayFlagsSkipClosestHitShaderEXT; // Will not invoke the hit shader, only the miss shader.
      isShadowed = true;
      traceRayEXT(topLevelAS,  // acceleration structure
            flags,       // rayFlags
            0xFF,        // cullMask
            1,           // sbtRecordOffset
            0,           // sbtRecordStride
            1,           // missIndex
            origin,      // ray origin
            tMin,        // ray min range
            rayDir,      // ray direction
            tMax,        // ray max range
            1            // payload (location = 1)
      );
}

