#version 410 core


in vec2 texCoords;
out vec4 fragColor;

uniform int numPositions;
uniform vec3 listPositions[400];
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;

vec2 projectToUV(vec3 p, out bool visible, out float depth)
{
    vec4 clip = projection * view * vec4(p, 1.0);
    visible = (clip.w > 0.0);
    depth = (clip.z / clip.w) * 0.5 + 0.5;
    vec3 ndc = clip.xyz / clip.w;
    return ndc.xy * 0.5 + 0.5;
}

void main()
{
    float minDistPx = 1e9;
    float depth = 1.0;

    vec2 fragPx = gl_FragCoord.xy;

    for (int i = 0; i < numPositions; i++)
    {
        float depthPoint = 1.0;
        bool visible;
        vec2 uv = projectToUV(listPositions[i], visible, depthPoint);
        if (!visible) continue;

        vec2 pointPx = uv * viewportSize;
        float dPx = length(fragPx - pointPx);

        if (dPx < minDistPx) {
            minDistPx = dPx;
            depth = depthPoint;
        }
    }

    float radiusPx = 5.0;

    if (minDistPx < radiusPx) {
        fragColor = vec4(1.0);
        gl_FragDepth = depth;
    } else {
        discard;
    }
}