#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <core/config.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <framework/window.h>

#include "BezierPath.h"

class Battlecruiser {
    Window &window;
    Config &config;

public:
    Battlecruiser(Window &window, Config &config);

    ~Battlecruiser();

    void draw(const glm::mat4 &view,
              const glm::mat4 &projection,
              const glm::vec3 &lightPos,
              const glm::vec3 &cameraPos,
              unsigned int cubemapTexture);
    void drawBezierPath(const glm::mat4 &view,
              const glm::mat4 &projection);

    void updatePosition(float deltaTime);

    std::vector<glm::vec3> getRelativePositionThrusters();

    glm::mat4 getModelMatrix();

    glm::vec3 getDirectionVector();

    glm::vec3 getUpVector();

    float getSpeed() const;

    virtual void imGuiControl();

private:
    BezierPath bezierPath;

    bool isFollowingPath = false;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    std::vector<glm::vec3> relativePositionThrusters = {
        glm::vec3(0.20f, -0.145f, -2.04f),
        glm::vec3(-0.022f, 0.077f, -2.04f),
        glm::vec3(-0.247f, -0.147f, -2.04f),
        glm::vec3(-0.022f, -0.372f, -2.04f),
    };

    float radius = 0.5f;
    int nrLights = 10;
    float thrusterRadius = 0.5f;
    float thrusterIntensity = 1.0f;

    Shader mainShader;
    Shader reflectiveShader;
    Shader thrusterShader;

    std::vector<MeshGL> meshGLs;

    static void loadTexture(const char *filename, GLuint &texture);
    void updateVelocityPositionFreeMovement(float deltaTime);
    void updateVelocityPositionPathMovement(float deltaTime);
};
