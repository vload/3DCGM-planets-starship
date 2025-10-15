#version 410 core
layout(vertices = 3) out;

in vec3 vsPosition[];
out vec3 tcsPosition[];

uniform vec3 cameraWorldPos;
uniform float tessMin = 1.0;
uniform float tessMax = 64.0;
uniform float lodRange = 10.0;
uniform float radius = 1.0;
uniform float targetPixelSize = 5.0; // desired triangle size in pixels
uniform float screenHeight = 1024.0;
uniform float fov = 1.3962; // vertical FOV in radians (~80 deg)

float computeTessLevel(vec3 p0, vec3 p1, vec3 p2)
{
    vec3 center = (p0 + p1 + p2) / 3.0;
    float dist = length(cameraWorldPos - center);

    float projectedSize = (radius / dist) * (screenHeight / (2.0 * tan(fov / 2.0)));

    float t = clamp(projectedSize / targetPixelSize, tessMin, tessMax);
    return t;
}

void main()
{
    tcsPosition[gl_InvocationID] = vsPosition[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        float t = computeTessLevel(vsPosition[0], vsPosition[1], vsPosition[2]);

        gl_TessLevelOuter[0] = t;
        gl_TessLevelOuter[1] = t;
        gl_TessLevelOuter[2] = t;
        gl_TessLevelInner[0] = max(1.0, t);
    }

    gl_out[gl_InvocationID].gl_Position = vec4(tcsPosition[gl_InvocationID], 1.0);
}