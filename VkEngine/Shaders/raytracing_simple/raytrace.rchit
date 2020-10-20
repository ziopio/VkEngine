#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#include "..\commons.glsl"
#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec3 attribs;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1, scalar) buffer SceneDesc {ObjDesc obj[];} sceneObjects;
layout(set = 0, binding = 2, scalar) buffer Vertices { Vertex3D vertices[]; } vertexBuffers[];
layout(set = 0, binding = 3) buffer Indices { uint indices[]; } indexBuffers[];
layout(set = 0, binding = 4) uniform sampler2D texSamplers[];

layout(set = 2, binding = 0)uniform uniBlock {
	mat4 P_inverted;
	mat4 V_inverted;
	Light lights[10];
	int light_count;
} uniforms;



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
  vec4 albedo = texture(texSamplers[object.textureId],UV);
  vec3  color = albedo.xyz / 10.0;
  // Point light
  if(uniforms.light_count >= 0)
  {
    vec3 lDir = normalize(uniforms.lights[0].position.xyz - worldPos);
    color += computeDiffuse(albedo.xyz, uniforms.lights[0], lDir, normal);
    color += computeSpecular(gl_WorldRayDirectionEXT, lDir, normal) * uniforms.lights[0].power.w;
  }

  hitValue = vec3(color);
}
