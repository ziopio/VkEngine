
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
  uint reflective;
};

#define Kc 1.0
#define Kl 0.35
#define Kq 0.44
#define LIGTH_ATTENUATION(dst) 1.0 / (Kc + Kl*dst + Kq*dst*dst)

struct Light{
	vec4 position;
	vec4 color;
	vec4 power;
};

bool facingLight(vec3 normal, vec3 light_dir){
  return dot(normal, light_dir) > 0;
}

vec3 computeDiffuse(vec3 matColor, Light light, vec3 lightDir, vec3 normal)
{
  // Lambertian
  float dotNL = max(dot(normal, lightDir), 0.0);
  return matColor * light.color.xyz * dotNL * light.power.w;
}

vec3 computeSpecular(Light light, vec3 viewDir, vec3 lightDir, vec3 normal)
{
  vec3        V                   = normalize(-viewDir);
  vec3        R                   = reflect(-lightDir, normal);
  float       specular            = pow(max(dot(V, R), 0.0), 100);
  return light.color.xyz * specular * light.power.w;
}
