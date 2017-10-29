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
	
	vec3 col = vec3(0.0f);
	vec3 N = texture(sampler2D(normalTexture, realTextureSampler), uv).rgb * 2.0f - 1.0f;
	N = normalize(TBN * N);
	vec3 V = normalize(myBufferVals.viewPos - fragPos);
	
	
	vec3 albedo = texture(sampler2D(albedoTexture, realTextureSampler), uv).rgb;
	albedo = vec3(pow(albedo.r, 2.2), pow(albedo.g, 2.2), pow(albedo.b, 2.2)); 
	float metallic = texture(sampler2D(metalTexture, realTextureSampler), uv).g;
	float roughness = texture(sampler2D(roughnessTexture, realTextureSampler), uv).r;
	float ao = texture(sampler2D(aoTexture, realTextureSampler), uv).r;

	vec3 F0 = vec3(0.4f);
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < myLightVals.currentAmountOfLights; ++i)
	{
		vec3 L =  normalize(vec3(myLightVals.lights[i].position) - fragPos);
		vec3 H = normalize(V + L);
		float distance = length(vec3(myLightVals.lights[i].position) - fragPos);
		float attenuation = 1.0f / (distance*distance);
		vec3 radiance = vec3(myLightVals.lights[i].color) * attenuation;

		float NDF = distributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H,V), 0.0), F0);
		
		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N,V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = nominator / denominator;

		vec3 kS = F;

		vec3 kD = vec3(1.0) - kS;

		kD *= 1.0f - metallic;

		float NdotL = max(dot(N, L), 0.0);

		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	vec3 ambient = vec3(0.03) * albedo;
	
	vec3 color = ambient + Lo;

	outColor = vec4(color.rgb, 1.0f);
	
	double intensity = dot(vec3(outColor.rgb), vec3(0.2126, 0.7152, 0.0722));

	if(intensity > 1.0f)
	{
		outBrightness = outColor;
	}
	
	else
	{
		outBrightness = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	
}