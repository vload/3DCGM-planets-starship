#version 410

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 spherePosition;
// in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;


vec3 lightPos = vec3(10.0, 0.0, 0.0);
uniform vec3 color = vec3(0.3, 0.3, 1.0);
uniform float time;
uniform float test = 0.0;
uniform float radius = 1.0;

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


// Parameters
int OCTAVES = 5;
float LACUNARITY = 2.0;
float PERSISTENCE = 0.45;

// Fractal 4D Simplex Noise
float fractalNoise(vec4 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxVal = 0.0;

    for (int i = 0; i < OCTAVES; i++) {
        value += snoise(p * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= PERSISTENCE;
        frequency *= LACUNARITY;
    }
    return value / maxVal;
}

// Optional: Turbulence (absolute value)
float turbulence(vec4 p) {
    return abs(fractalNoise(p));
}

// Optional: Domain warping
vec4 warp(vec4 p) {
    float wx = fractalNoise(p * 1.5 + vec4(0.0, 0.0, 0.0, 0.0));
    float wy = fractalNoise(p * 1.5 + vec4(100.0, 100.0, 100.0, 100.0));
    return p + vec4(wx, wy, wx, wy) * 0.5;
}

float coloreh(vec4 pos) {
    // Domain warp for more natural cells
    pos = warp(pos);

    // float noise_val = turbulence(pos);
    float noise_val = fractalNoise(pos);

    // Optional post-processing
    // noise_val = noise_val * 0.5 + 0.5; // map from [-1,1] to [0,1]
    // noise_val = pow(noise_val, 2.4); // increase contrast
    // noise_val = noise_val * 2.5 - 1.0; // map from [0,1] to [-1,1]
    // noise_val = clamp(noise_val, -1.0, 1.0);


    return noise_val;

    // Map [-1,1] -> [0,1]
    // noise_val = noise_val * 0.5 + 0.5;

    // // Color mapping with a smooth blend
    // vec3 colorFromNoise;
    // if (noise_val <= 0.5) {
    //     float t = smoothstep(0.0, 0.5, noise_val);
    //     colorFromNoise = mix(c0, c1, t);
    // } else {
    //     float t = smoothstep(0.5, 1.0, noise_val);
    //     colorFromNoise = mix(c1, c2, t);
    // }

    // fragColor = vec4(colorFromNoise, 1.0);
}

// Computes a procedural ocean normal for a spherical planet
vec3 getOceanNormal(vec3 pos, float time) {
    float delta = 0.01;
    float oceanScale = 10.0;
    float speed = 0.3;
    // Construct tangent space on the sphere
    vec3 up = abs(pos.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, pos));
    vec3 bitangent = normalize(cross(pos, tangent));
    
    // Sample height from 4D fractal noise
    float h = fractalNoise(vec4(pos * oceanScale, time * speed));
    float h_t = fractalNoise(vec4((pos + tangent * delta) * oceanScale, time * speed));
    float h_b = fractalNoise(vec4((pos + bitangent * delta) * oceanScale, time * speed));

    // Compute gradient in tangent plane
    vec3 grad = tangent * (h_t - h) + bitangent * (h_b - h);
    
    // Perturb the spherical normal
    vec3 normal = normalize(pos + grad);
    return normal;
}

uniform vec3 cameraWorldPos;


void main()
{
    // vec3 lightDir = normalize(lightPos - fragPosition);
    // vec3 normal = normalize(fragNormal);
    // float diffuse = max(dot(normal, lightDir), 0.0);
    // fragColor = vec4(color * diffuse, 1.0);
    // float noise_val = snoise(vec4(spherePosition * 20.0, time));
    float noise_val = coloreh(vec4(spherePosition * 20.0, time * 0.3));
    // noise_val changes:
    // noise_val = noise_val * 0.5 + 0.5; // map from [-1,1] to [0,1]
    // noise_val = noise_val * 4.0; // scale to [0,4]
    // noise_val = pow(noise_val, 3.0); // increase contrast
    // noise_val = clamp(noise_val, 0.0, 50.0);
    // noise_val = noise_val / 50.0; // map from [0,50] to [0,1]
    // noise_val = -noise_val * 2.0 + 1.0; // map from [0,1] to [-1,1]

    vec3 c0 = vec3(0.5, 0.0, 0.0);
    vec3 c1 = vec3(1.0, 0.5, 0.0);
    vec3 c2 = vec3(1.0, 1.0, 1.0);

    vec3 colorFromNoise;
    if (noise_val <= 0.0) {
        float t = clamp(noise_val + 1.0, 0.0, 1.0); // map [-1,0] -> [0,1]
        colorFromNoise = mix(c0, c1, t);
    } else {
        float t = clamp(noise_val, 0.0, 1.0); // map [0,1] -> [0,1]
        colorFromNoise = mix(c1, c2, t);
    }

    fragColor = vec4(colorFromNoise, 1.0);
    // spherePosition = normalize(pos) * (1.0 + height * test);

    if (length(spherePosition) < 1.0) {
        float x = (length(spherePosition) - 1.0) / test;
        vec3 waterColor = vec3(0.0, 0.0, 1.0);
        vec3 col1 = vec3(0.933, 0.933, 1.0);   // #EEF
        vec3 col2 = vec3(0.133, 0.333, 0.467); // #257
        vec3 col3 = vec3(0.067, 0.133, 0.267); // #124
        
        if (x >= -0.1) {
            // interpolate between col1 and col2 from 0 → -0.1
            float t = clamp((x - 0.0) / (-0.1 - 0.0), 0.0, 1.0);
            waterColor = mix(col1, col2, t);
        } else {
            // interpolate between col2 and col3 from -0.1 → -1.0
            float t = clamp((x - (-0.1)) / (-1.0 - (-0.1)), 0.0, 1.0);
            waterColor = mix(col2, col3, t);
        }

        // waves using octaves of sine waves and domain warping
        float ka = 0.1; // ambient
        float kd = 0.9; // diffuse
        float ks = 0.9; // specular

        vec3 normal = getOceanNormal(spherePosition, time);
        vec3 lightDir = normalize(lightPos - fragPosition);

        // Phong reflection model
        vec3 ambient = ka * waterColor.rgb;
        vec3 diffuse = kd * max(dot(normal, lightDir), 0.0) * waterColor.rgb;
        vec3 viewDir = normalize(cameraWorldPos - fragPosition);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 specular = ks * pow(max(dot(viewDir, reflectDir), 0.0), 128.0) * vec3(1.0);

        fragColor = vec4(ambient + diffuse + specular, 1.0);
        // fragColor = vec4(normal, 1.0);
    }
    else{
        float x = (length(spherePosition) - 1.0) / test;
        if(x <= 0.25){
            fragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
        else if(x <= 0.4){
            fragColor = vec4(0.5, 0.5, 0.5, 1.0);
        }
        else{
            fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        // fragColor = vec4(vec3(0.0, 1.0 - ((length(spherePosition) - 1.0) / test), 0.0), 1.0);
    }
}
