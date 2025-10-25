#version 410 core

// Output for on-screen color.
out vec4 outColor;

// Interpolated output data from vertex shader.
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

uniform samplerCube environmentMap;
uniform vec3 cameraPos;

void main()
{
    vec3 I = normalize(fragPos - cameraPos);
    vec3 R = reflect(I, normalize(fragNormal));
    vec3 envColor = texture(environmentMap, R).rgb;

    // Blend with subtle glass tint
    vec3 glassTint = vec3(0.8, 0.9, 1.0);
    outColor = vec4(mix(glassTint, envColor, 0.7), 0.6);
}