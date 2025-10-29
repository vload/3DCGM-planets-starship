#version 410 core

// Model/view/projection matrix
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Per-vertex attributes
layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;

// Data to pass to fragment shader
out vec4 fragWorldPos;
out vec3 fragPos;
out vec3 fragNormal;

void main() {
	// Transform 3D position into on-screen position
    gl_Position = projection * view * model * vec4(pos, 1.0);

    // Pass position and normal through to fragment shader
    fragWorldPos = model * vec4(pos, 1.0);
    fragPos = pos;
    fragNormal = normal;
}