#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include "ParticleSystem.h"
#include <framework/window.h>

class Battlecruiser {
    Window& window;
public:
    Battlecruiser(Window& window);
    ~Battlecruiser();

    void draw(const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& lightPos,
        const glm::vec3& cameraPos,
        unsigned int cubemapTexture);

    void updateVelocityPosition(float deltaTime);

    std::vector<glm::vec3> getRelativePositionThrusters();
    glm::mat4 getModelMatrix();

    glm::vec3 getDirectionVector();
    glm::vec3 getUpVector();

private:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));

    std::vector<glm::vec3> relativePositionThrusters = {
        glm::vec3(0.0f, -0.5f, -21.0f),
        glm::vec3(0.0f, 8.5f, -21.0f),
        glm::vec3(4.5f, 4.0f, -21.0f),
        glm::vec3(-4.5f, 4.0f, -21.0f)
    };

    Shader mainShader;
    Shader reflectiveShader;

    std::vector<MeshGL> meshGLs;
    LightParticle thruster;
};