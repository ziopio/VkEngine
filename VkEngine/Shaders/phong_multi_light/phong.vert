#version 450
#extension GL_ARB_separate_shader_objects:enable

struct Light{
	vec4 position;
	vec4 color;
	vec4 power;
};

layout(location=0)in vec3 inPosition;
layout(location=1)in vec3 inNormal;
layout(location=2)in vec3 inColor;
layout(location=3)in vec2 inTexCoord;

layout(location=0)out vec3 fragColor;
layout(location=1)out vec2 fragTexCoord;
layout(location=2)flat out int outTextureIndex;
layout(location=3)out vec3 Position_worldspace;
layout(location=4)out vec3 Normal_cameraspace;
layout(location=5)out vec3 EyeDirection_cameraspace;
layout(location=6)out vec3 LightDirection_cameraspace[10];

layout(set=1,binding=0)uniform uniBlock{
	mat4 P;
	mat4 V;
	Light lights[10];
	int light_count;
}uniforms;
layout(push_constant)uniform PushConsts
{
	// cpu computed
	mat4 M;// 64 bytes (vec4 *4)
	// the texture position for the current 3D mesh
	int textureIndex;// 4 bytes
}pushConsts;

void main(){
	gl_Position=uniforms.P*uniforms.V*
	pushConsts.M*vec4(inPosition,1.);
	fragColor=inColor;
	outTextureIndex=pushConsts.textureIndex;
	fragTexCoord=inTexCoord;
	
	Position_worldspace=(pushConsts.M*
		vec4(inPosition,1)).xyz;
		
	vec3 vertexPosition_cameraspace=(uniforms.V*
		pushConsts.M*vec4(inPosition,1)).xyz;
		
	EyeDirection_cameraspace=-vertexPosition_cameraspace;
			
	// Vector that goes from the vertex to the light, in camera space.
	// M is omited because it's identity.
	vec3 LightPosition_cameraspace;
	for(int i=0;i<uniforms.light_count;i++){
		LightPosition_cameraspace=(uniforms.V*
		uniforms.lights[i].position).xyz;
		LightDirection_cameraspace[i]=LightPosition_cameraspace+
		EyeDirection_cameraspace;
	}
	// Normal of the the vertex, in camera space
	//Normal_cameraspace = ( V * M * vec4(vertexNormal_modelspace,0)).xyz;
	// Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	Normal_cameraspace=transpose(inverse(mat3(uniforms.V*pushConsts.M)))*
		inNormal;
}