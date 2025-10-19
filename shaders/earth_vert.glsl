#version 410

// uniform mat4 mvpMatrix;
// uniform mat4 modelMatrix;
// // Normals should be transformed differently than positions:
// // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
// uniform mat3 normalModelMatrix;

layout(location = 0) in vec3 position;
// layout(location = 1) in vec3 normal;
// layout(location = 2) in vec2 texCoord;

out vec3 vsPosition;
// out vec3 fragNormal;
// out vec2 fragTexCoord;

void main()
{
    vsPosition = position;
    // gl_Position is ignored here; TES will compute it
    // gl_Position = mvpMatrix * vec4(position, 1);
    
    // fragPosition    = (modelMatrix * vec4(position, 1)).xyz;
    // fragNormal      = normalModelMatrix * normal;
    // fragTexCoord    = texCoord;
}
