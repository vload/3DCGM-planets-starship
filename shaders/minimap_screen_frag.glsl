#version 410 core

in vec2 fragUV;

out vec4 outColor;

uniform sampler2D minimapTex;

void main()
{
    vec4 texColor = texture(minimapTex, fragUV);
    outColor = texColor;
}