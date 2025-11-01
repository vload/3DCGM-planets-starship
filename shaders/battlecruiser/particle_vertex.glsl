#version 410 core

// 4 corners of the quad: (-0.5,-0.5,0) .. (+0.5,+0.5,0)
layout(location = 0) in vec3 quadCorner;

// Per-instance
layout(location = 1) in vec4 inPositionSize;   // xyz = center, w = size
layout(location = 2) in vec4 inColor;          // normalized from GL_UNSIGNED_BYTE

out vec4 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;

void main()
{
    vec3 center = inPositionSize.xyz;
    float size  = inPositionSize.w;

    // Build billboard in world space from camera basis and per-vertex corner
    vec3 worldPos = center.xyz
                  + CameraRight_worldspace * quadCorner.x * size
                  + CameraUp_worldspace    * quadCorner.y * size;

    gl_Position = projection * view * vec4(worldPos, 1.0);
    vColor = inColor;
}