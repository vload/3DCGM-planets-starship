#version 410 core

layout(location = 0) in vec3 pos;

out vec2 texCoords;

void main() {
    gl_Position = vec4(pos, 1.0);

    texCoords = pos.xy * 0.5 + 0.5;
}