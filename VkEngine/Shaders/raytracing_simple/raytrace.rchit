#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "..\commons.glsl"
#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec3 attribs;

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1) buffer SceneDesc {ObjDesc obj[];} sceneObjects;
layout(set = 0, binding = 2) buffer Vertices { Vertex3D vertices[]; } vertexBuffers[];
layout(set = 0, binding = 3) buffer Indices { uint indices[]; } indexBuffers[];

void main()
{
  // Object of this instance
  uint mesh_id = sceneObjects.obj[gl_InstanceID].meshId;

  //Indices of the triangle
  ivec3 index = ivec3(indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 0], 
                      indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 1], 
                      indexBuffers[nonuniformEXT(mesh_id)].indices[3 * gl_PrimitiveID + 2]);


  hitValue = vec3(index);

}
