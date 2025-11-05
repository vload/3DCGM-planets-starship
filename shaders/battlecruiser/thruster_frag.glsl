#version 410 core

#define M_PI 3.1415926535897932384626433832795

uniform mat4 model;

in vec3 fragPos;
in vec3 fragWorldPos;
in vec3 fragNormal;

out vec4 outColor;

uniform vec3 thrusterLightPos; // in model space
uniform vec3 thrusterLightColor;
uniform vec3 thrusterDir; // in world space

uniform float radius;
uniform int nrLights;
uniform float thrusterRadius;
uniform float thrusterIntensity;

void main() {
    // Transform thruster position to world space
    vec3 thrusterWorldPos = vec3(model * vec4(thrusterLightPos, 1.0));

    // Use world-space direction as-is
    vec3 lightNormal = normalize(thrusterDir);

    // Build orthonormal basis in world space
    vec3 vectorA = vec3(0.0f, 0.0f, 1.0f);
    if (lightNormal.x <= lightNormal.y && lightNormal.x <= lightNormal.z)
        vectorA = vec3(1.0f, 0.0f, 0.0f);
    else
        if (lightNormal.y <= lightNormal.x && lightNormal.y < lightNormal.z)
            vectorA = vec3(0.0f, 1.0f, 0.0f);

    vec3 u = normalize(cross(lightNormal, vectorA));
    vec3 v = cross(lightNormal, u);

    vec3 result = vec3(0.0f);

    for (int j = 1; j <= 3; j++) {
        float radiusLight = mix(0.0f, radius, float(j) / 3);

        for (int i = 0; i < nrLights; i++) {

            float angle = (2.0 * M_PI * float(i)) / float(nrLights);
            vec3 lightPos = thrusterWorldPos + radiusLight * (cos(angle) * u + sin(angle) * v);

            // Distance-based falloff float
            float dist = length(fragWorldPos - lightPos);
            float attenuation = 1.0 - clamp(dist / thrusterRadius, 0.0, 1.0);
            attenuation *= attenuation;

            vec3 L = normalize(lightPos - fragWorldPos);
            vec3 lightEffect = thrusterLightColor * thrusterIntensity * attenuation * max(dot(fragNormal, L), 0.0);

            result += lightEffect;
        }
    }
    // Centre light
    vec3 lightPos = thrusterWorldPos;

    // Distance-based falloff
    float dist = length(fragWorldPos - lightPos);
    float attenuation = 1.0 - clamp(dist / thrusterRadius, 0.0, 1.0);
    attenuation *= attenuation;

    vec3 L = normalize(lightPos - fragWorldPos);
    vec3 lightEffect = thrusterLightColor * thrusterIntensity * attenuation * max(dot(fragNormal, L), 0.0);
    result += lightEffect; outColor = vec4(result, 1.0);
}