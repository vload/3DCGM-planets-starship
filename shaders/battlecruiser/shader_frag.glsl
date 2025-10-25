#version 410 core

// scene uniforms
uniform vec3 lightPos;

// ThrusterLight uniforms
uniform vec3 thrusterLightPos;
uniform float thrusterThresholdLight;
uniform float thrusterLightAngle;
uniform vec3 thrusterLightDir;
uniform vec3 thrusterLightColor;
uniform float thrusterLightIntensity;

// Output for on-screen color.
out vec4 outColor;

// Interpolated output data from vertex shader.
in vec4 fragWorldPos;
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

float computeThrusterLight(vec3 fragPos)
{
    // Distance attenuation
    float dist = distance(fragPos, thrusterLightPos);
    if (dist > thrusterThresholdLight)
        return 0.0;

    // Direction from thruster to fragment
    vec3 L = normalize(fragPos - thrusterLightPos);

    // Angular falloff
    float angleCos = dot(normalize(-thrusterLightDir), L);
    float cutoffCos = cos(thrusterLightAngle);
    if (angleCos < cutoffCos)
        return 0.0;

    // Smooth falloff near cone edges and by distance
    float distAtt = 1.0 - dist / thrusterThresholdLight;
    float angleAtt = smoothstep(cutoffCos, 1.0, angleCos);

    return distAtt * angleAtt * thrusterLightIntensity;
}

void main()
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragWorldPos.xyz);

    // Directional (main) light
    float diff = max(dot(N, L), 0.0);
    vec3 baseLight = diff * vec3(1.0); // white main light

    // Thruster light
    float thrusterFactor = computeThrusterLight(fragPos);
    vec3 thrusterContribution = thrusterLightColor * thrusterFactor;

    // Combine both lights
    vec3 finalColor = baseLight + thrusterContribution;

    outColor = vec4(finalColor, 1.0);
}