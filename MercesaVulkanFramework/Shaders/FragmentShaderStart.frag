#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 0, binding = 1) uniform sampler realTextureSampler;
layout(set = 2, binding = 0) uniform texture2D realTexture;


layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 fragPos;


layout (location = 0) out vec4 outColor;

void main() {

	vec3 lightDir = normalize(vec3(0.0f, 0.5f, 0.5));
	float test = max(dot(lightDir, normal), 0.0f);
	outColor = texture(sampler2D(realTexture, realTextureSampler), uv) * test;
}