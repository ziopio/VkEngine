#version 450
#extension GL_ARB_separate_shader_objects : enable

 struct Light{	 
	vec4 position;
	vec4 color;
	vec4 power;
 };

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2)flat in int inTextureIndex;

layout(location = 3)in vec3 Position_worldspace;
layout(location = 4)in vec3 Normal_cameraspace;
layout(location = 5)in vec3 EyeDirection_cameraspace;
layout(location = 6)in vec3 LightDirection_cameraspace[10];

layout(set = 0, binding = 0) uniform sampler2D texSamplers[1024];
layout(set = 0, binding = 1) uniform uniBlock{
    mat4 P_matrix;
    mat4 V_matrix;
    Light lights[10];
	int light_count;
} uniforms;

layout(location = 0) out vec4 outColor;

float att = 1.0, att_linear = 0.000000000000001, att_quadratic = 0.01;

void main() {

    float distance;
	float cosAlpha, cosTheta;
	vec3 n,l,E,R;

    outColor = vec4(0.0,0.0,0.0,1.0);
	vec4 texel = texture(texSamplers[inTextureIndex], fragTexCoord);
	for(int i = 0; i < uniforms.light_count ; i++){
		// Distance to the light
		distance = length( uniforms.lights[i].position.xyz - Position_worldspace);
		// Normal of the computed fragment, in camera space
		n = normalize( Normal_cameraspace );
		// // Direction of the light (from the fragment to the light)
		l = normalize( LightDirection_cameraspace[i] );
		// Cosine of the angle between the normal and the light direction, 
		// clamped above 0
		//  - light is at the vertical of the triangle -> 1
		//  - light is perpendicular to the triangle -> 0
		//  - light is behind the triangle -> 0
		cosTheta = clamp( dot( n, l ), 0,1 );
		// Eye vector (towards the camera)
		E = normalize(EyeDirection_cameraspace);
		// Direction in which the triangle reflects the light
		R = reflect(-l,n);
		// Cosine of the angle between the Eye vector and the Reflect vector,
		// clamped to 0
		//  - Looking into the reflection -> 1
		//  - Looking elsewhere -> < 1
		cosAlpha = clamp( dot( E,R ), 0,1 );
		//fragColor = vec3(1,1,0);

		outColor += 
		// Ambient : simulates indirect lighting
		vec4(0.001,0.001,0.001,1.0) +
		// Diffuse : "color" of the object
		texel * uniforms.lights[i].power.w * cosTheta / (att + att_linear*distance + att_quadratic*distance*distance ) +
		// Specular : reflective highlight, like a mirror
		vec4(1.0,1.0,1.0,1.0) * uniforms.lights[i].power.w * pow(cosAlpha,500) / (distance*distance);
	}
}