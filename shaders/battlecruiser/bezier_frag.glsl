#version 410 core

in vec2 texCoords;
out vec4 fragColor;

uniform int numCurves;
uniform vec3 curves[400];
uniform mat4 view;
uniform mat4 projection;

vec2 projectToUV(vec3 p, out bool visible, out float depth)
{
    vec4 clip = projection * view * vec4(p, 1.0);
    visible = (clip.w > 0.0);
    depth = (clip.z / clip.w) * 0.5 + 0.5;
    vec3 ndc = clip.xyz / clip.w;
    return ndc.xy * 0.5 + 0.5;
}

float distanceToBezier(vec2 uv, vec3 p0, vec3 p1, vec3 p2, vec3 p3, out float depth)
{
    float minDist = 1.0;
    depth = 1.0;
    for (int i = 0; i <= 32; ++i)
    {
        float t = float(i) / 32.0;
        float u = 1.0 - t;
        float tt = t * t;
        float uu = u * u;
        vec3 pos = uu*u*p0 + 3.0*uu*t*p1 + 3.0*u*tt*p2 + tt*t*p3;
        bool visible;
        float dDepth;
        vec2 b = projectToUV(pos, visible, dDepth);
        if (!visible) continue;
        float d = length(uv - b);
        if (d < minDist) {
            minDist = d;
            depth = dDepth;
        }
    }
    return minDist;
}

void main()
{
    float minDist = 1.0;
    float depth = 1.0;
    for (int i = 0; i < numCurves; ++i)
    {
        vec3 p0 = curves[i*4 + 0];
        vec3 p1 = curves[i*4 + 1];
        vec3 p2 = curves[i*4 + 2];
        vec3 p3 = curves[i*4 + 3];
        float dDepth;
        float d = distanceToBezier(texCoords, p0, p1, p2, p3, dDepth);
        if (d < minDist) {
            minDist = d;
            depth = dDepth;
        }
    }

    float thickness = 0.005;
    float alpha = smoothstep(thickness, thickness * 0.5, minDist);
    fragColor = vec4(vec3(1.0), alpha);
    gl_FragDepth = depth;
}