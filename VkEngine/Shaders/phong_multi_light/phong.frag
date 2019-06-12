#version 450
#extension GL_ARB_separate_shader_objects:enable

struct Light{
	vec4 position;
	vec4 color;
	vec4 power;
};

layout(location=0)in vec3 fragColor;
layout(location=1)in vec2 fragTexCoord;
layout(location=2)flat in int inTextureIndex;

layout(location=3)in vec3 Position_worldspace;
layout(location=4)in vec3 Normal_cameraspace;
layout(location=5)in vec3 EyeDirection_cameraspace;
layout(location=6)in vec3 LightDirection_cameraspace[10];

layout(set=0,binding=0)uniform sampler2D texSamplers[32];
layout(set=1,binding=0)uniform uniBlock{
	mat4 P;
	mat4 V;
	Light lights[10];
	int light_count;
}uniforms;

layout(location=0)out vec4 outColor;

#define linear_coef .01
#define quadratic_coef .01

void main(){
	
	float distance;
	float cosAlpha,cosTheta;
	vec3 N,L,E,R;
	
	vec4 texel=texture(texSamplers[inTextureIndex],fragTexCoord);	

	outColor= texel * 0.05;

	for(int i=0;i<uniforms.light_count;i++){
		// Distance to the light
		distance=length(uniforms.lights[i].position.xyz-
		Position_worldspace);
		float attenuation=1.;//1.0 /
		// 	( 1.0 + linear_coef * distance  +
		// 	quadratic_coef * (distance*distance));
		// Normal of the computed fragment, in camera space
		N=normalize(Normal_cameraspace);
		// // Direction of the light (from the fragment to the light)
		L=normalize(LightDirection_cameraspace[i]);
		// Cosine of the angle between the normal and the light direction,
		cosTheta=max(dot(N,L),0.);
		// Eye vector (towards the camera)
		E=normalize(EyeDirection_cameraspace);
		// Direction in which the triangle reflects the light
		R=reflect(-L,N);
		// Cosine of the angle between the Eye vector and the Reflect vector,
		cosAlpha=max(dot(E,R),0.);
		//fragColor = vec3(1,1,0);
		
		outColor+=
		// Diffuse : "color" of the object
		texel * uniforms.lights[i].power.w * cosTheta * attenuation +
		// Specular : reflective highlight, like a mirror
		vec4(1.0,1.0,1.0,1.0) * uniforms.lights[i].power.w *
			pow(cosAlpha,100) * attenuation;
	}
	outColor = vec4 (outColor.rgb ,texel.a); // alpha correction
}