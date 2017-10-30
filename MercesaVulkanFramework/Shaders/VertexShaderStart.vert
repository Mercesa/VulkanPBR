#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Light
{
	// vec4's since if we choose to encode more data, we are able to.
	vec4 position;
	vec4 color;
	vec4 specularColor;
};

layout(std140, set = 3, binding = 0) uniform modelBuffer{
	mat4 modelMatrix;
} mySingleUniform;

layout(std140, set = 3, binding = 1) uniform pbrDataBuffer{
	float metallic;
	float roughness;
	int shouldUse;
} pbrMaterialData;



layout(std140, set = 1, binding = 0) uniform bufferVals{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	mat4 viewProjectionMatrix;
	mat4 mvp;
} myBufferVals;

layout(std140, set = 1, binding = 1) uniform lightVals{
	Light lights[16];
	int currentAmountOfLights;
} myLightVals;




layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec3 tangent;


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outFragPos;
layout(location = 4) out mat3 outTBN;


void main() {
	outColor = vec4(uv.xy, 0.0, 1.0);
	outUV = uv;
	outNormal = normalize(mat3x3(mySingleUniform.modelMatrix) * normal);
	outFragPos = vec3(mySingleUniform.modelMatrix * vec4(pos, 1.0));

	vec3 T = normalize(vec3(mySingleUniform.modelMatrix * vec4(tangent, 0.0f)));
	vec3 B = normalize(vec3(mySingleUniform.modelMatrix * vec4(bitangent, 0.0f)));
	vec3 N = normalize(vec3(mySingleUniform.modelMatrix * vec4(normal, 0.0f)));

	outTBN = mat3(T,B,N);

	gl_Position = myBufferVals.viewProjectionMatrix * mySingleUniform.modelMatrix * vec4(pos, 1.0);


}
