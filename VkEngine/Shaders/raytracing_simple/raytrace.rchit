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

void main()
{
  // Object of this instance
  uint mesh_id = sceneObjects.obj[gl_InstanceID].meshId;

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
  vec3 normal = v0.nrm.xyz * barycentrics.x + v1.nrm.xyz * barycentrics.y + v2.nrm.xyz * barycentrics.z;


  hitValue = vec3(normal);

}
