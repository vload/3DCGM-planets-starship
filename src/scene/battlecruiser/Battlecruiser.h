#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <core/config.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <framework/window.h>

#include "BezierPath.h"
#include "../bodies/PlanetSystem.h"

class Battlecruiser {
    Window &window;
    Config &config;

    ShadowMap battlecruiserShadowMap1;
    ShadowMap battlecruiserShadowMap2;

public:
    Battlecruiser(Window &window, Config &config);

    ~Battlecruiser();

    void draw_shadow(PlanetSystem& planetSystem);

    void draw(const glm::mat4 &view,
              const glm::mat4 &projection,
              const glm::vec3 &lightPos,
              const glm::vec3 &cameraPos,
              unsigned int cubemapTexture,
              PlanetSystem& planetSystem);
    void drawBezierPath(const glm::mat4 &view,
              const glm::mat4 &projection);

    void updatePosition(float deltaTime);

    void updateLights(PlanetSystem& system);

    std::vector<glm::vec3> getRelativePositionThrusters();

    glm::mat4 getModelMatrix();

    glm::vec3 getDirectionVector();

    glm::vec3 getUpVector();

    glm::vec3 getPosition() const;

    float getSpeed() const;

    std::vector<glm::vec3> getBezierSampledPoints();

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

    // light params
    glm::mat4 light_view_matrix1;
    glm::vec3 light_position1;
    glm::mat4 light_projection_matrix1;
    glm::mat4 light_view_matrix2;
    glm::vec3 light_position2;
    glm::mat4 light_projection_matrix2;

    void draw_shadowmap(PlanetSystem& planetSystem, ShadowMap& shadowMap, glm::mat4& lightViewMatrix, glm::mat4& lightProjectionMatrix);

    static void loadTexture(const char *filename, GLuint &texture);
    void updateVelocityPositionFreeMovement(float deltaTime);
    void updateVelocityPositionPathMovement(float deltaTime);
};
