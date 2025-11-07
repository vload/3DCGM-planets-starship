#version 410 core

layout (location = 0) in vec3 position;

out vec3 texCoords;

// view/projection matrix
uniform mat4 mvp;

void main() {
	vec4 pos = mvp * vec4(position, 1.0f);
	gl_Position = pos.xyww;
	texCoords = position;
}