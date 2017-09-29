#version 400
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
} myBufferVals;

layout(std140, set = 1, binding = 1) uniform lightVals{
	Light lights[16];
	int currentAmountOfLights;
} myLightVals;



layout(set = 0, binding = 0) uniform sampler realTextureSampler;
layout(set = 2, binding = 0) uniform texture2D realTexture;


layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 fragPos;


layout (location = 0) out vec4 outColor;

void main() {

	vec3 col = vec3(0.0f);
	for(int i = 0; i < myLightVals.currentAmountOfLights; ++i)
	{

	}
		vec3 lightDir =  vec3(myLightVals.lights[0].position) - fragPos;
		float lengthL = length(lightDir);
		lightDir = normalize(lightDir);

		float d = max(dot(lightDir, normal), 0.0f);
		col += d * vec3(myLightVals.lights[0].color);// * (1.0f/(1.0f+1.0f*lengthL));
	//vec3 lightCol = vec3(myLightVals.lights[0].color);
	outColor = texture(sampler2D(realTexture, realTextureSampler), uv) * vec4(col.rgb, 1.0f);
}