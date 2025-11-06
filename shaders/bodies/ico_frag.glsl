#version 410

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

in vec3 fragPosition;
in vec3 spherePosition;

layout(location = 0) out vec4 fragColor;


vec3 lightPos = vec3(10.0, 0.0, 0.0);
uniform vec3 body_color = vec3(0.3, 0.3, 1.0);
uniform float time;
uniform vec3 planet_center;
uniform float radius;


// Eclipse "ray tracing"
uniform int enable_eclipse = 1;
uniform int num_bodies = 0;
uniform vec4 bodyPosRadii[128]; // xyz = position, w = radius of all bodies

uniform int binary_system = 0; // 0 = single star, 1 = binary star
uniform vec4 sunPosRadii[128]; // xyz = position, w = radius of light sources
uniform vec4 sunColInt[128]; // xyz = color, w = intensity

uniform float default_ka = 0.1;
uniform float default_kd = 1.0;

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
        if (length(planet_center - bodyPos) < 0.01) continue;   // skip self-eclipse
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

        // eclipse logic could be improved with umbra/penumbra model, but good enough for now
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

uniform float exposure; // tweak this
uniform float gamma; // typically 2.2 for sRGB monitors

void main()
{
    float covered1 = eclipse_factor(fragPosition, sunPosRadii[0]);
    float eclipseLightFactor1 = 1.0 - covered1;
    float eclipseLightFactor2 = 1.0;
    if(binary_system == 1) {
        float covered2 = eclipse_factor(fragPosition, sunPosRadii[1]);
        eclipseLightFactor2 = 1.0 - covered2;
    }

    vec3 normal = normalize(spherePosition); // approximate normal on sphere
    vec3 ambient = default_ka * body_color;
    vec3 hdrColor = ambient;

    vec3 lightDir1 = normalize(lightPosition1 - fragPosition);
    vec3 diffuse1 = default_kd * max(dot(normal, lightDir1), 0.0) * body_color;
    hdrColor += diffuse1 * eclipseLightFactor1 * sunColInt[0].w * sunColInt[0].xyz * exposure;

    if(binary_system == 1) {
        vec3 lightDir2 = normalize(lightPosition2 - fragPosition);
        vec3 diffuse2 = default_kd * max(dot(normal, lightDir2), 0.0) * body_color;
        hdrColor += diffuse2 * eclipseLightFactor2 * sunColInt[1].w * sunColInt[1].xyz * exposure;
    }

    // Reinhard tone mapping
    vec3 mappedColor = hdrColor / (hdrColor + vec3(1.0));

    // Gamma correction
    mappedColor = pow(mappedColor, vec3(1.0/gamma));

    fragColor = vec4(mappedColor, 1.0);
}
