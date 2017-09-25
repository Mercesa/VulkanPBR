#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D realTextureSampler;

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 0) out vec4 outColor;

void main() {

	
	vec3 lightDir = normalize(vec3(1.0f, -0.5f, 0.99f));

	float test = max(dot(lightDir, normal), 0.0f);
   outColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * test;
}