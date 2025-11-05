#version 410 core

in vec2 uv;

out vec4 outColor;

uniform vec3 planetColor;

uniform int isPlanet;

void main() {
    float dist = length(uv);

    if (isPlanet == 1) {
        if (dist > 1.0)
        discard;
    }
    else {
        if (abs(uv.x) > 1.0 || abs(uv.y) > 1.0)
        discard;
    }

    outColor = vec4(planetColor, 1.0);
}