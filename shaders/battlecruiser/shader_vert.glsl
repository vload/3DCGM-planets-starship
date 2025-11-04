#version 410 core

// Model/view/projection matrix
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Per-vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texCoord;
layout(location=3) in vec3 tangent;

// Data to pass to fragment shader
out vec3 fragWorldPos;
out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out mat3 tbn;

void main() {
	// Transform 3D position into on-screen position
    gl_Position = projection * view * model * vec4(position, 1.0);

    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    vec3 B = normalize(cross(N, T));

    // Pass position and normal through to fragment shader
    fragWorldPos = vec3(model * vec4(position, 1.0));
    fragPos = position;
    fragNormal = N;
    fragTexCoord = texCoord;
    tbn = mat3(T, B, N);
}