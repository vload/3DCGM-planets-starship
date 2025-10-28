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
uniform vec3 color = vec3(0.3, 0.3, 1.0);
uniform float time;
uniform vec3 planet_center;
uniform float radius;


// Eclipse "ray tracing"
uniform int enable_eclipse = 1;

uniform int num_bodies = 0;
uniform vec4 bodyPosRadii[128]; // xyz = position, w = radius of all bodies


uniform int binary_system = 0; // 0 = single star, 1 = binary star
uniform vec4 sunPosRad;
// uniform vec4 sunPosRad2;

// Projects point P0 onto a plane defined by point P1 and normal N
vec3 projectPointOntoPlane(vec3 P0, vec3 P1, vec3 N) {
    vec3 v = P0 - P1;
    float d = dot(v, N) / dot(N, N); // If N is normalized, just use dot(v, N)
    return P0 - d * N;
}

float eclipse_factor(vec3 fragPos, vec4 sunPosRad) {
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

void main()
{
    float covered = 0.0;
    if (enable_eclipse > 0) {
        covered = eclipse_factor(fragPosition, sunPosRad);
    }
    float eclipseLightFactor = 1.0 - covered;

    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 normal = normalize(spherePosition); // approximate normal on sphere
    float diffuse = max(dot(normal, lightDir), 0.0);

    float ambient = 0.1;

    fragColor = vec4(color * (diffuse * eclipseLightFactor + ambient), 1.0);
}
