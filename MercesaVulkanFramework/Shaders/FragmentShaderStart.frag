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
layout(set = 2, binding = 1) uniform texture2D specularTexture;
layout(set = 2, binding = 2) uniform texture2D normalTexture;



layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 fragPos;
layout (location = 4) in mat3 TBN;


layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outSecond;


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {

	vec3 col = vec3(0.0f);
	vec3 newNormal = texture(sampler2D(normalTexture, realTextureSampler), uv).rgb * 2.0f - 1.0f;
	newNormal = normalize(TBN * newNormal);

	for(int i = 0; i < myLightVals.currentAmountOfLights; ++i)
	{
		vec3 lightDir =  vec3(myLightVals.lights[i].position) - fragPos;


		float lengthL = length(lightDir);
		lightDir = normalize(lightDir);
		float d = max(dot(lightDir, newNormal), 0.0f);

		
		vec3 viewDir = normalize(myBufferVals.viewPos - fragPos);
		vec3 H = normalize(lightDir+viewDir); 

		vec3 reflectDir = reflect(-lightDir, newNormal);
		float spec = pow(max(dot(normal, H), 0.0f), 16);
		vec3 specColor = vec3(1.0f) * spec * (1.0f/(1.0f+4.0f*lengthL)) * texture(sampler2D(specularTexture, realTextureSampler), uv).rgb;
		col += d * vec3(myLightVals.lights[i].color) * (1.0f/(1.0f+4.0f*lengthL));
		col += specColor;
	}
		

		
	//vec3 lightCol = vec3(myLightVals.lights[0].color);
	//outColor = texture(sampler2D(realTexture, realTextureSampler), uv) * vec4(col.rgb, 1.0f);
	outColor = texture(sampler2D(albedoTexture, realTextureSampler), uv) + vec4(col.rgb, 1.0f);

	outSecond = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}