#version 430
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D textureScene;
layout(set = 0, binding = 1) uniform sampler2D textureBloom;


void main()
{
	vec3 color =   vec3(texture(textureScene, inUV));
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));	

	outColor = vec4(color.rgb, 1.0f);
}