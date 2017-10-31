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

layout(std140, set = 1, binding = 0) uniform bufferVals{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;

	mat4 viewProjectionMatrix;
	mat4 mvp;

	vec3 viewPos;

} myBufferVals;

layout(std140, set = 1, binding = 1) uniform lightVals{
	Light lights[16];
	int currentAmountOfLights;
} myLightVals;





layout(set = 0, binding = 0) uniform sampler realTextureSampler;

layout(set = 2, binding = 0) uniform texture2D albedoTexture;
layout(set = 2, binding = 1) uniform texture2D metalTexture;
layout(set = 2, binding = 2) uniform texture2D normalTexture;
layout(set = 2, binding = 3) uniform texture2D roughnessTexture;
layout(set = 2, binding = 4) uniform texture2D aoTexture;



layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in mat3 TBN;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outBrightness;


void main() {
	
	outColor = vec4(sin(gl_FragCoord.x), cos(gl_FragCoord.y), 0.0f, 1.0f);
}