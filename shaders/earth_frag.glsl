#version 410

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

float permute(float x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
{
    const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
    vec4 p,s;

    p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
    p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
    s = vec4(lessThan(p, vec4(0.0)));
    p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www; 

    return p;
}
						
// (sqrt(5) - 1)/4 = F4, used once below
#define F4 0.309016994374947451

float snoise(vec4 v)
{
    const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                        0.276393202250021,  // 2 * G4
                        0.414589803375032,  // 3 * G4
                        -0.447213595499958); // -1 + 4 * G4

    // First corner
    vec4 i  = floor(v + dot(v, vec4(F4)) );
    vec4 x0 = v -   i + dot(i, C.xxxx);

    // Other corners

    // Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
    vec4 i0;
    vec3 isX = step( x0.yzw, x0.xxx );
    vec3 isYZ = step( x0.zww, x0.yyz );
    //  i0.x = dot( isX, vec3( 1.0 ) );
    i0.x = isX.x + isX.y + isX.z;
    i0.yzw = 1.0 - isX;
    //  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
    i0.y += isYZ.x + isYZ.y;
    i0.zw += 1.0 - isYZ.xy;
    i0.z += isYZ.z;
    i0.w += 1.0 - isYZ.z;

    // i0 now contains the unique values 0,1,2,3 in each channel
    vec4 i3 = clamp( i0, 0.0, 1.0 );
    vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
    vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

    //  x0 = x0 - 0.0 + 0.0 * C.xxxx
    //  x1 = x0 - i1  + 1.0 * C.xxxx
    //  x2 = x0 - i2  + 2.0 * C.xxxx
    //  x3 = x0 - i3  + 3.0 * C.xxxx
    //  x4 = x0 - 1.0 + 4.0 * C.xxxx
    vec4 x1 = x0 - i1 + C.xxxx;
    vec4 x2 = x0 - i2 + C.yyyy;
    vec4 x3 = x0 - i3 + C.zzzz;
    vec4 x4 = x0 + C.wwww;

    // Permutations
    i = mod289(i); 
    float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
    vec4 j1 = permute( permute( permute( permute (
                i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
            + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
            + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
            + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

    // Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
    // 7*7*6 = 294, which is close to the ring size 17*17 = 289.
    vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

    vec4 p0 = grad4(j0,   ip);
    vec4 p1 = grad4(j1.x, ip);
    vec4 p2 = grad4(j1.y, ip);
    vec4 p3 = grad4(j1.z, ip);
    vec4 p4 = grad4(j1.w, ip);

    // Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    p4 *= taylorInvSqrt(dot(p4,p4));

    // Mix contributions from the five corners
    vec3 m0 = max(0.57 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
    vec2 m1 = max(0.57 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
    m0 = m0 * m0;
    m1 = m1 * m1;
    return 60.1 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
                + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 spherePosition;
in float height;

layout(location = 0) out vec4 fragColor;


uniform float ocean_level = 0.0;
// TODO: this is not working properly yet and should be a uniform
vec3 lightPos = vec3(0.0, 0.0, 0.0);
uniform vec3 color = vec3(0.3, 0.3, 1.0);
uniform float time;
uniform float test = 0.0;
uniform float radius = 1.0;
uniform vec3 planet_center = vec3(0.0, 0.0, 0.0);

uniform float ocean_scale = 10.0;
uniform float ocean_speed = 0.3;

// Parameters
uniform int OCTAVES_water = 5;
uniform float LACUNARITY_water = 2.0;
uniform float PERSISTENCE_water = 0.45;

// Fractal 4D Simplex Noise
float fractalNoise(vec4 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxVal = 0.0;

    for (int i = 0; i < OCTAVES_water; i++) {
        value += snoise(p * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= PERSISTENCE_water;
        frequency *= LACUNARITY_water;
    }
    return value / maxVal;
}


// Computes a procedural ocean normal for a spherical planet
// TODO: the normals are not quite right, but close enough for now
vec3 getOceanNormal(vec3 pos, float time) {
    float delta = 0.01;
    // Construct tangent space on the sphere
    vec3 up = abs(pos.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, pos));
    vec3 bitangent = normalize(cross(pos, tangent));
    
    // Sample height from 4D fractal noise
    float h = fractalNoise(vec4(pos * ocean_scale, time * ocean_speed));
    float h_t = fractalNoise(vec4((pos + tangent * delta) * ocean_scale, time * ocean_speed));
    float h_b = fractalNoise(vec4((pos + bitangent * delta) * ocean_scale, time * ocean_speed));

    // Compute gradient in tangent plane
    vec3 grad = tangent * (h_t - h) + bitangent * (h_b - h);
    
    // Perturb the spherical normal
    vec3 normal = normalize(pos + grad);
    return normal;
}

uniform vec3 cameraWorldPos;
uniform float waterKa = 0.2; // ambient
uniform float waterKd = 0.8; // diffuse
uniform float waterKs = 0.9; // specular
uniform float waterShininess = 128.0;

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

    if (height < ocean_level) {
        float ocean_depth = ocean_level - height;
        vec3 waterColor = vec3(0.0, 0.0, 1.0);
        
        // TODO: gradient color based on depth (could be uniform params)
        vec3 col1 = vec3(0.933, 0.933, 1.0);   // #EEF
        vec3 col2 = vec3(0.133, 0.333, 0.467); // #257
        vec3 col3 = vec3(0.067, 0.133, 0.267); // #124

        if (ocean_depth <= 0.1) {
            // interpolate between col1 and col2 from 0 → -0.1
            float t = clamp(ocean_depth / 0.1, 0.0, 1.0);
            waterColor = mix(col1, col2, t);
        } else {
            float t = clamp((ocean_depth - 0.1) / (1.0 + ocean_level - 0.1), 0.0, 1.0);
            waterColor = mix(col2, col3, t);
        }

        vec3 normal = getOceanNormal(spherePosition, time);
        vec3 lightDir = normalize(lightPos - fragPosition);

        // Phong reflection model
        vec3 ambient = waterKa * waterColor.rgb;
        vec3 diffuse = waterKd * max(dot(normal, lightDir), 0.0) * waterColor.rgb;
        vec3 viewDir = normalize(cameraWorldPos - fragPosition);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 specular = waterKs * pow(max(dot(viewDir, reflectDir), 0.0), waterShininess) * vec3(1.0);

        fragColor = vec4(ambient + (diffuse + specular) * eclipseLightFactor, 1.0);
    }
    else{
        //TODO: maybe use materials for this, or at least blinnphong with uniforms
        float t = (height - ocean_level) / (1.0 - ocean_level);
        vec3 green = vec3(0.0, 1.0, 0.0);
        vec3 gray  = vec3(0.5, 0.5, 0.5);
        vec3 white = vec3(1.0, 1.0, 1.0);

        vec3 col;
        if (t <= 0.2) {
            col = green;
        } else if (t < 0.3) {
            float s = smoothstep(0.2, 0.3, t);
            col = mix(green, gray, s);
        } else if (t <= 0.37) {
            col = gray;
        } else if (t < 0.43) {
            float s = smoothstep(0.37, 0.43, t);
            col = mix(gray, white, s);
        } else {
            col = white;
        }

        // TODO:
        // temp phong shading (the tessellation does not provide normals yet)
        float ka = 0.1; // ambient
        float kd = 0.9; // diffuse

        vec3 normal = spherePosition; // approximate normal on sphere
        vec3 lightDir = normalize(lightPos - fragPosition);

        // Phong reflection model
        vec3 ambient = ka * col;
        vec3 diffuse = kd * max(dot(normal, lightDir), 0.0) * col;

        fragColor = vec4(ambient + diffuse * eclipseLightFactor, 1.0);
    }
}
