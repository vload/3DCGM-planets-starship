#version 410 core
layout(triangles, equal_spacing, ccw) in;

in vec3 tcsPosition[];
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 spherePosition;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float radius = 1.0;

void main()
{
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    vec3 pos = gl_TessCoord.x * p0 +
               gl_TessCoord.y * p1 +
               gl_TessCoord.z * p2;

    spherePosition = normalize(pos);
    pos = normalize(pos) * radius;

    fragNormal = normalize(pos);

    gl_Position = projection * view * model * vec4(pos, 1.0);
    fragPosition = (model * vec4(pos, 1.0)).xyz;
}
