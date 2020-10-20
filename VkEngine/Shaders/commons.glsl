
struct Vertex3D
{
  vec3 pos;
  vec3 nrm;
  vec3 color;
  vec2 texCoord;
};

struct ObjDesc
{
  uint meshId;
  uint textureId;
  mat4 transform;
};

struct Light{
	vec4 position;
	vec4 color;
	vec4 power;
};

vec3 computeDiffuse(vec3 matColor, Light light,vec3 lightDir, vec3 normal)
{
  // Lambertian
  float dotNL = max(dot(normal, lightDir), 0.0);
  return matColor * dotNL * light.power.w;
}

vec3 computeSpecular( vec3 viewDir, vec3 lightDir, vec3 normal)
{
  vec3        V                   = normalize(-viewDir);
  vec3        R                   = reflect(-lightDir, normal);
  float       specular            = pow(max(dot(V, R), 0.0), 100);
  return vec3(1.,1.,1.) * specular;
}