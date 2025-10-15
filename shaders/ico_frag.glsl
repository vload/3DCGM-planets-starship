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
// in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;


uniform vec3 lightDir = normalize(vec3(10.0, 0.0, 0.0));
uniform vec3 color = vec3(0.3, 0.3, 1.0);

void main()
{
    // vec3 normal = normalize(fragNormal);
    // fragColor = vec4(normal, 1);

    float diffuse = max(dot(fragNormal, lightDir), 0.0);
    fragColor = vec4(color * diffuse, 1.0);
}
