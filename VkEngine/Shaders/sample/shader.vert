#version 450
#extension GL_ARB_separate_shader_objects : enable

 struct Light{     
	vec3 position;
	vec3 color;
	float power;
 };

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2)flat out int textureIndex;

layout(set = 1, binding = 0) uniform uniBlock{
    mat4 P_matrix;
    mat4 V_matrix;
    int light_count;
    Light lights[10];
} uniforms;
layout (push_constant) uniform PushConsts 
{
    // cpu computed
	mat4 model_trasform;  // 64 bytes (vec4 *4)
    // the texture position for the current 3D mesh
    int textureIndex; // 4 bytes
} pushConsts;

void main() {  
    gl_Position = uniforms.P_matrix * uniforms.V_matrix * pushConsts.model_trasform * vec4(inPosition, 1.0);    
    //fragColor = inColor;
    fragTexCoord = inTexCoord;
    textureIndex = pushConsts.textureIndex;
}