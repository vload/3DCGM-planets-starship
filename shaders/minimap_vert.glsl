#version 410 core

layout (location = 0) in vec2 position;

uniform mat4 projection;
uniform vec2 planetPos;
uniform float planetScale;

out vec2 uv;

void main() {
    uv = position;

    vec2 pos = planetPos + position * planetScale;

    gl_Position = projection * vec4(pos, 0.0, 1.0);
}