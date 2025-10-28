#version 410
//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)//, out vec3 gradient)
{
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  vec4 m2 = m * m;
  vec4 m4 = m2 * m2;
  vec4 pdotx = vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3));

// Determine noise gradient
  vec4 temp = m2 * m * pdotx;
  // gradient = -8.0 * (temp.x * x0 + temp.y * x1 + temp.z * x2 + temp.w * x3);
  // gradient += m4.x * p0 + m4.y * p1 + m4.z * p2 + m4.w * p3;
  // gradient *= 105.0;

  return 105.0 * dot(m4, pdotx);
}

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// uniform float radius = 1.0;
// uniform float test = 0.0;
uniform float shape_noise_scale = 0.1;

// Earth shape parameters
// uniform float ocean_level = 0.0;

uniform float shape_noise_base_frequency = 1.2;
uniform float shape_noise_pseudo_seed = 100.0;

// Parameters
uniform int OCTAVES_shape = 5;
uniform float LACUNARITY_shape = 2.0;
uniform float PERSISTENCE_shape = 0.45;

// Fractal 3D Simplex Noise
float fractalNoise(vec3 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;
    float maxVal = 0.0;

    for (int i = 0; i < OCTAVES_shape; i++) {
        value += snoise(p * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= PERSISTENCE_shape;
        frequency *= LACUNARITY_shape;
    }
    return value / maxVal;
}

float get_height(vec3 sphere_pos) {
    return fractalNoise(sphere_pos * shape_noise_base_frequency + vec3(shape_noise_pseudo_seed));
}

vec3 get_surface_normal(vec3 pos)
{
    float eps = 0.001; // small step for finite differences
    float h = get_height(pos);

    float hx = get_height(pos + vec3(eps, 0.0, 0.0));
    float hy = get_height(pos + vec3(0.0, eps, 0.0));
    float hz = get_height(pos + vec3(0.0, 0.0, eps));

    vec3 grad = vec3(hx - h, hy - h, hz - h) / eps;
    
    // Normal points opposite to gradient, then normalize
    vec3 n = normalize(pos - grad * shape_noise_scale);
    return n;
}

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

// vec4 mod289(vec4 x) {
//   return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

// vec4 permute(vec4 x) {
//      return mod289(((x*34.0)+10.0)*x);
// }

float permute(float x) {
     return mod289(((x*34.0)+10.0)*x);
}

// vec4 taylorInvSqrt(vec4 r)
// {
//   return 1.79284291400159 - 0.85373472095314 * r;
// }

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
in vec3 spherePosition;
in float height;

layout(location = 0) out vec4 fragColor;


uniform float ocean_level = 0.0;
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


uniform float ocean_normal_gradient_multiplier = 0.01;

// Computes a procedural ocean normal for a spherical planet
vec3 getOceanNormal(vec3 pos, float time) {
    float eps = 0.001; // small step for finite differences
    float delta = 0.001;
    
    float h = fractalNoise(vec4(pos * ocean_scale, time * ocean_speed));

    float hx = fractalNoise(vec4((pos + vec3(1.0, 0.0, 0.0) * delta) * ocean_scale, time * ocean_speed));
    float hy = fractalNoise(vec4((pos + vec3(0.0, 1.0, 0.0) * delta) * ocean_scale, time * ocean_speed));
    float hz = fractalNoise(vec4((pos + vec3(0.0, 0.0, 1.0) * delta) * ocean_scale, time * ocean_speed));

    // Compute gradient in tangent plane
    vec3 grad = vec3(hx - h, hy - h, hz - h) / eps;
    
    // Normal points opposite to gradient, then normalize
    vec3 n = normalize(pos - grad * ocean_normal_gradient_multiplier);
    return n;
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

uniform vec3 lightPosition;
uniform mat4 lightViewMatrix;
uniform mat4 lightProjectionMatrix;
uniform int only_depth = 0;

uniform int enable_shadowmapping = 1;
uniform int enable_PCF = 1;
uniform int PCF_kernel_radius = 1;

uniform sampler2D shadowMap;

float shadow_calculation(vec3 fragPos, vec3 normal, vec3 lightDir)
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

    float covered = 0.0;
    if (enable_eclipse > 0) {
        covered = eclipse_factor(fragPosition, sunPosRad);
    }
    float eclipseLightFactor = 1.0 - covered;

    // Shadows
    float shadowMultiplier = 1.0;
    // shadowMultiplier = 1.0 - shadow_calculation(fragPosition);

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
        vec3 lightDir = normalize(lightPosition - fragPosition);

        // Phong reflection model
        vec3 ambient = waterKa * waterColor.rgb;
        vec3 diffuse = waterKd * max(dot(normal, lightDir), 0.0) * waterColor.rgb;
        vec3 viewDir = normalize(cameraWorldPos - fragPosition);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 specular = waterKs * pow(max(dot(viewDir, reflectDir), 0.0), waterShininess) * vec3(1.0);

        shadowMultiplier = 1.0 - shadow_calculation(fragPosition, normal, lightDir);
        fragColor = vec4(ambient + (diffuse + specular) * eclipseLightFactor * shadowMultiplier, 1.0);
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

        // TODO: temp phong shading, replace with PBR later
        float ka = 0.1; // ambient
        float kd = 0.9; // diffuse

        vec3 normal = get_surface_normal(spherePosition);
        vec3 lightDir = normalize(lightPosition - fragPosition);

        // Phong reflection model
        vec3 ambient = ka * col;
        vec3 diffuse = kd * max(dot(normal, lightDir), 0.0) * col;
        
        shadowMultiplier = 1.0 - shadow_calculation(fragPosition, normal, lightDir);
        fragColor = vec4(ambient + diffuse * eclipseLightFactor * shadowMultiplier, 1.0);
    }
    }
    else{
        // do nothing for depth-only pass
    }
}
