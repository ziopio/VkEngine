

struct hitPayload
{ 
  uint stop;
  uint depth;
  vec3 attenuation;
  vec3 rayOrigin;
  vec3 rayDir;
  vec4 hitValue;
};

struct shadowPayload
{
  float shadow_alpha;
};


