#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding = 0) uniform bufferVals{
	mat4 mvp;
} myBufferVals;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec3 tangent;


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;

void main() {
	outColor = vec4(uv.xy, 0.0, 1.0);
	outUV = uv;
	gl_Position = myBufferVals.mvp * vec4(pos, 1.0);
}
