#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}