

// struct Vertex3D
// {
//   float posx; float posy; float posz;
//   float nrmx; float nrmy; float nrmz;
//   float colorR; float colorG; float colorB;
//   float texCoordU; float texCoordV;
// };

struct Vertex3D
{
  vec3 pos;
  vec3 nrm;
  vec3 color;
  vec2 texCoord;
};

struct ObjDesc
{
  uint  meshId;
  uint  textureId;
};