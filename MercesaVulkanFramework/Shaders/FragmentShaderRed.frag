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
layout (location = 1) out vec4 outSecond;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N,H), 0.0);
	float NdotH2 = NdotH*NdotH;
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r*r) / 8.0f;
	
	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom/denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void main() {
	
	outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}