#version 410 core

#define M_PI 3.1415926535897932384626433832795

// scene uniforms
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

uniform sampler2D textureBase;
uniform sampler2D textureNormal;
uniform sampler2D textureMetallic;
uniform sampler2D textureRoughness;

// Output for on-screen color.
out vec4 outColor;

in vec3 fragWorldPos;
in vec3 fragPos;
in vec2 fragTexCoord;
in mat3 tbn;

float dotProduct(vec3 X, vec3 Y) {
    return max(dot(X, Y), 0.0);
}

float fresnel(float f0, vec3 V, vec3 H) {
    return f0 + (1 - f0) * pow(1 - dotProduct(V, H), 5);
}

vec3 diffuseLambert(vec3 color) {
    return color / M_PI;
}

vec3 specularCook(float f0, float roughness, vec3 V, vec3 L, vec3 N, vec3 H) {
    float alpha = pow(roughness, 2);

    float D = pow(alpha, 2) / (M_PI * pow(pow(dotProduct(N, H), 2) * (pow(alpha, 2) - 1) + 1, 2));
    float G =
        (dotProduct(N, L) / max((dotProduct(N, L) * (1.0 - (roughness / 2.0)) + (roughness / 2.0)), 0.000001)) *
        (dotProduct(N, V) / max((dotProduct(N, V) * (1.0 - (roughness / 2.0)) + (roughness / 2.0)), 0.000001));
    float F = fresnel(f0, V, H);

    return (D * G * F) / max((4.0 * dotProduct(V, N) * dotProduct(L, N)), 0.000001);
}

void main()
{
    vec3 baseColor = texture(textureBase, fragTexCoord).rgb;
    vec3 normalMap = texture(textureNormal, fragTexCoord).rgb;
    float metallic = texture(textureMetallic, fragTexCoord).r;
    float roughness = texture(textureRoughness, fragTexCoord).r;
    float f0 = 0.4;

    normalMap = normalize(tbn * (normalMap * 2.0 - 1.0));

    vec3 L = normalize(lightPos - fragWorldPos);
    vec3 N = normalMap;
    vec3 V = normalize(cameraPos - fragWorldPos);
    vec3 H = normalize(L + V);

    float kd = (1 - fresnel(f0, V, H)) * (1 - metallic);

    vec3 result = lightColor * dotProduct(L, N) * (kd * diffuseLambert(baseColor) + specularCook(f0, roughness, V, L, N, H));

    outColor = vec4(result, 1.0);
}