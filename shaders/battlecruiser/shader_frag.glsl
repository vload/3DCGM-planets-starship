#version 410 core

#define M_PI 3.1415926535897932384626433832795

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform vec3 center;

uniform sampler2D textureBase;
uniform sampler2D textureNormal;
uniform sampler2D textureMetallic;
uniform sampler2D textureRoughness;

out vec4 outColor;

in vec3 fragWorldPos;
in vec3 fragPos;
in vec2 fragTexCoord;
in mat3 tbn;

uniform int usePBRShader;

float dotProduct(vec3 X, vec3 Y) {
    return max(dot(X, Y), 0.0);
}

vec3 fresnel(vec3 f0, vec3 V, vec3 H) {
    return f0 + (1.0 - f0) * pow(1.0 - dotProduct(V, H), 5.0);
}

vec3 diffuseLambert(vec3 color) {
    return color / M_PI;
}

vec3 specularCook(vec3 f0, float roughness, vec3 V, vec3 L, vec3 N, vec3 H) {
    float alpha = pow(roughness, 2.0);

    float D = pow(alpha, 2.0) / (M_PI * pow(pow(dotProduct(N, H), 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0));
    float G =
    (dotProduct(N, L) / max((dotProduct(N, L) * (1.0 - (roughness / 2.0)) + (roughness / 2.0)), 0.000001)) *
    (dotProduct(N, V) / max((dotProduct(N, V) * (1.0 - (roughness / 2.0)) + (roughness / 2.0)), 0.000001));
    vec3 F = fresnel(f0, V, H);

    return (D * G * F) / max((4.0 * dotProduct(V, N) * dotProduct(L, N)), 0.000001);
}

// Eclipse "ray tracing"
uniform int enable_eclipse = 1;
uniform int enable_shadowmapping = 1;
uniform int enable_PCF = 1;
uniform int PCF_kernel_radius = 1;
uniform float exposure; // tweak this
uniform float gamma;

uniform int num_bodies = 0;
uniform vec4 bodyPosRadii[128]; // xyz = position, w = radius of all bodies

uniform int binary_system = 0; // 0 = single star, 1 = binary star
uniform vec4 sunPosRadii[128]; // xyz = position, w = radius of light sources
uniform vec4 sunColInt[128]; // xyz = color, w = intensity

// Projects point P0 onto a plane defined by point P1 and normal N
vec3 projectPointOntoPlane(vec3 P0, vec3 P1, vec3 N) {
    vec3 v = P0 - P1;
    float d = dot(v, N) / dot(N, N); // If N is normalized, just use dot(v, N)
    return P0 - d * N;
}

float eclipse_factor(vec3 fragPos, vec4 sunPosRad) {
    if(enable_eclipse == 0) {
        return 0.0;
    }
    vec3 sunPos = sunPosRad.xyz;
    float sunRad = sunPosRad.w;

    float covered = 0.0;
    vec3 toLight = sunPos - fragPos;
    float dist_to_light = length(toLight);
    vec3 lightDir = normalize(toLight);

    for (int i = 0; i < num_bodies; i++) {
        vec3 bodyPos = bodyPosRadii[i].xyz;
        float bodyRadius = bodyPosRadii[i].w;

        // Sanity checks
        if (bodyRadius <= 0.0) continue;                        // skip degenerate bodies
        if (length(center - bodyPos) < 0.01) continue;   // skip self-eclipse
        if (length(sunPos - bodyPos) < 0.01) continue;          // skip if body is the light source

        vec3 toBody = bodyPos - fragPos;
        if(dot(toLight, toBody) <= 0.0) continue;               // body is behind fragment relative to light

        float dist_to_body = length(toBody);
        if(dist_to_body > dist_to_light) continue;              // body is beyond the light source
        
        // Compute angular radii
        float sunAngle  = asin(sunRad / dist_to_light);
        float bodyAngle = asin(bodyRadius / dist_to_body);

        // Angle between sun and body
        float angleBetween = acos(dot(normalize(toLight), normalize(toBody)));

        // TODO: eclipse logic could be improved with umbra/penumbra model, but good enough for now
        // Check for overlap
        if (angleBetween < (bodyAngle + sunAngle)) {
            // Fractional coverage approximation
            float overlap = clamp((bodyAngle + sunAngle - angleBetween) / (2.0 * sunAngle), 0.0, 1.0);
            covered = 1.0 - (1.0 - covered) * (1.0 - overlap);
        }
    }

    return clamp(covered, 0.0, 1.0);
}

uniform vec3 lightPosition1;
uniform mat4 lightViewMatrix1;
uniform mat4 lightProjectionMatrix1;
uniform vec3 lightPosition2;
uniform mat4 lightViewMatrix2;
uniform mat4 lightProjectionMatrix2;
uniform int only_depth = 0;

uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;

float shadow_calculation(vec3 fragPos, vec3 normal, vec3 lightDir, mat4 lightViewMatrix, mat4 lightProjectionMatrix, sampler2D shadowMap)
{
    if(enable_shadowmapping == 0) {
        return 0.0;
    }
    // 1. Transform fragment to light space
    vec4 fragPosLightSpace = lightProjectionMatrix * lightViewMatrix * vec4(fragPos, 1.0);
    
    // 2. Perspective divide and map to [0,1] texture coords
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // 3. Outside shadow map = fully lit
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // 4. PCF setup
    float shadow = 0.0;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005); // angle-dependent bias
    // float bias = 0.005;
    ivec2 texSize = textureSize(shadowMap, 0); 
    vec2 texelSize = 1.0 / vec2(texSize);

    int kernel_radius = PCF_kernel_radius;
    if(enable_PCF == 0) {
        kernel_radius = 0;
    }

    // 5. Sample 3x3 neighborhood
    for(int x = -kernel_radius; x <= kernel_radius; ++x)
    {
        for(int y = -kernel_radius; y <= kernel_radius; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            float closestDepth = texture(shadowMap, projCoords.xy + offset).r;
            if(projCoords.z - bias > closestDepth)
                shadow += 1.0;
        }
    }

    shadow /= float((kernel_radius * 2 + 1) * (kernel_radius * 2 + 1)); // average over samples
    return shadow; // 0.0 = fully lit, 1.0 = fully shadowed
}

void main()
{
    if(only_depth != 1) {

    float covered1 = eclipse_factor(fragWorldPos, sunPosRadii[0]);
    float eclipseLightFactor1 = 1.0 - covered1;
    float eclipseLightFactor2 = 1.0;
    if(binary_system == 1) {
        float covered2 = eclipse_factor(fragWorldPos, sunPosRadii[1]);
        eclipseLightFactor2 = 1.0 - covered2;
    }

    vec3 baseColor = texture(textureBase, fragTexCoord).rgb;
    vec3 normalMap = texture(textureNormal, fragTexCoord).rgb;
    float metallic = texture(textureMetallic, fragTexCoord).r;
    float roughness = texture(textureRoughness, fragTexCoord).r;

    baseColor = pow(baseColor, vec3(2.2)); // convert from sRGB to linear space

    normalMap = normalize(tbn * (normalMap * 2.0 - 1.0));
    vec3 L1 = normalize(lightPosition1 - fragWorldPos);
    vec3 N = normalMap;
    vec3 V = normalize(cameraPos - fragWorldPos);
    vec3 f0 = mix(vec3(0.04), baseColor, metallic);

    vec3 resultColorPbr = baseColor;

    if (usePBRShader == 1) {
        vec3 H1 = normalize(L1 + V);

        vec3 F1 = fresnel(f0, V, H1);
        float kd1 = (1.0 - max(max(F1.r, F1.g), F1.b)) * (1.0 - metallic);
        vec3 diffuse1 = kd1 * diffuseLambert(baseColor);

        vec3 specular1 = specularCook(f0, roughness, V, L1, N, H1);

        resultColorPbr = diffuse1 + specular1;
    }

    vec3 hdrColor = vec3(0.0);
    float shadowMultiplier1 = 1.0 - shadow_calculation(fragWorldPos, normalMap, L1, lightViewMatrix1, lightProjectionMatrix1, shadowMap1);

    hdrColor += (eclipseLightFactor1 * shadowMultiplier1) * sunColInt[0].rgb * sunColInt[0].w * dotProduct(L1, N) * resultColorPbr * exposure;

    if(binary_system == 1) {
        vec3 resultColorPbr2 = baseColor;
        vec3 L2 = normalize(lightPosition2 - fragWorldPos);

        if (usePBRShader == 1) {

            vec3 H2 = normalize(L2 + V);

            vec3 F2 = fresnel(f0, V, H2);
            float kd2 = (1.0 - max(max(F2.r, F2.g), F2.b)) * (1.0 - metallic);
            vec3 diffuse2 = kd2 * diffuseLambert(baseColor);

            vec3 specular2 = specularCook(f0, roughness, V, L2, N, H2);

            resultColorPbr2 = diffuse2 + specular2;
        }


        float shadowMultiplier2 = 1.0 - shadow_calculation(fragWorldPos, normalMap, L2, lightViewMatrix2, lightProjectionMatrix2, shadowMap2);


        hdrColor += (eclipseLightFactor2 * shadowMultiplier2) * sunColInt[1].rgb * sunColInt[1].w * dotProduct(L2, N) * resultColorPbr2 * exposure;
    }
    // No tone mapping or gamma correction, because our desired effect is to have very bright highlights
    outColor = vec4(hdrColor, 1.0);
    } else {
        // do nothing for depth-only pass
    }
}
