#include "SkyboxSystem.h"
#include <stb/stb_image.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

SkyboxSystem::SkyboxSystem()
    : skyboxVertices{
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f
    },
    skyboxIndices{
        1, 6, 2, 
        6, 1, 5, 
        0, 7, 4, 
        7, 0, 3, 
        4, 6, 5, 
        6, 4, 7, 
        0, 2, 3, 
        2, 0, 1, 
        0, 5, 1, 
        5, 0, 4, 
        3, 6, 7, 
        6, 3, 2, 
    }
{
    // Load cube-map
    std::vector<std::string> skyboxFaces =
    {
        RESOURCE_ROOT "resources/skybox/px.png", // +X (right)
        RESOURCE_ROOT "resources/skybox/nx.png", // -X (left)
        RESOURCE_ROOT "resources/skybox/py.png", // +Y (top)
        RESOURCE_ROOT "resources/skybox/ny.png", // -Y (bottom)
        RESOURCE_ROOT "resources/skybox/pz.png", // +Z (front)
        RESOURCE_ROOT "resources/skybox/nz.png"  // -Z (back)
    };

    _cubemapTexture = loadCubemap(skyboxFaces);

    // Create VAO, VBO, and EBO for the skybox
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    unsigned int skyboxVBO, skyboxEBO;
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SkyboxSystem::draw(const glm::mat4& view, const glm::mat4& projection, const Shader &shader) {
    glDepthFunc(GL_LEQUAL);
    shader.bind();

    glm::mat4 skyboxMVP = projection * glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(shader.getUniformLocation("mvp"), 1, GL_FALSE, glm::value_ptr(skyboxMVP));
    glUniform1i(shader.getUniformLocation("skybox"), 0);

    glBindVertexArray(_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTexture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

unsigned int SkyboxSystem::getCubemapTexture() {
    return _cubemapTexture;
}

SkyboxSystem::~SkyboxSystem() {
    glDeleteVertexArrays(1, &_vao);
}

unsigned int SkyboxSystem::loadCubemap(const std::vector<std::string>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_uc* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 3);
        if (data)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, // which face
                0,
                GL_RGB,
                width,
                height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Failed to load cubemap face: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // Set filtering and wrapping
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}