#version 410 core
layout(triangles, equal_spacing, ccw) in;

in vec3 tcsPosition[];
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 spherePosition;

uniform mat4 mvpMatrix;
uniform float radius = 1.0;
uniform float test = 0.0;

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

// Parameters
int OCTAVES = 5;
float LACUNARITY = 2.0;
float PERSISTENCE = 0.45;

// Fractal 3D Simplex Noise
float fractalNoise(vec3 p) {
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


uniform float ocean_depth = 2.0;

void main()
{
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 pos = gl_TessCoord.x * p0 +
               gl_TessCoord.y * p1 +
               gl_TessCoord.z * p2;

    spherePosition = normalize(pos);

    vec3 gradient;
    // float noise_val = snoise(spherePosition * 10, gradient);

    // float continent_noise = snoise(spherePosition * 1.5 + vec3(100.0), gradient);
    // if(continent_noise < 0.0) { // oceans
    //     pos = normalize(pos) * (radius + continent_noise * ocean_depth);
    //     spherePosition = pos / radius;
    //     pos = normalize(pos) * (radius); // flatten ocean surface
    // }
    // else {
    //     float mountain_noise = snoise(spherePosition * 3.0 + vec3(200.0), gradient);
    //     mountain_noise = 2 * (1 - abs(mountain_noise)) - 1; // ridge
    //     if(mountain_noise < 0.3) {
    //         pos = normalize(pos) * (radius + continent_noise * test);
    //         spherePosition = pos / radius;
    //     }
    //     else {
    //         pos = normalize(pos) * (radius + continent_noise * test + mountain_noise * test);
    //         spherePosition = pos / radius;
    //     }
    //     float ridge_noise_val = snoise(spherePosition * 0.1 , gradient);
    //     ridge_noise_val = abs(ridge_noise_val);
    //     ridge_noise_val = 1 - ridge_noise_val;
    //     ridge_noise_val = ridge_noise_val * ridge_noise_val;
    //     // ridge_noise_val = pow(ridge_noise_val, 5.0); // gain

    //     if(ridge_noise_val > 0.5) {// mountains
    //         // noise_val = pow(noise_val, ridge_noise_val * 2.0);
    //         pos = normalize(pos) * (radius + test);
    //     }
    //     else{
    //         pos = normalize(pos) * (radius + 0.1);
    //     }
        

    //     // pos = normalize(pos) * (radius + noise_val * test);
        
    //     spherePosition = pos / radius;
    // }
    float height = fractalNoise(spherePosition * 5.0 + vec3(100.0));
    spherePosition = normalize(pos) * (1.0 + height * test);
    if(length(spherePosition) < 1.0) {
        pos = normalize(pos) * radius; // flatten ocean surface
    }
    else{
        pos = spherePosition * radius;
    }


    fragNormal = normalize(pos); // TODO: compute better normal with gradient

    gl_Position = mvpMatrix * vec4(pos, 1.0);
    fragPosition = gl_Position.xyz;
}
