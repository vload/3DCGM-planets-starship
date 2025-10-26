#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <framework/shader.h>

class SkyboxSystem {
public:
    SkyboxSystem();
    ~SkyboxSystem();

    void draw(const glm::mat4& view, const glm::mat4& projection, const Shader& shader);
    unsigned int getCubemapTexture();

private:
    unsigned int loadCubemap(const std::vector<std::string>& faces);

    const float skyboxVertices[24];
    const unsigned int skyboxIndices[36];

    GLuint _vao = 0;
    unsigned int _cubemapTexture = 0;
};