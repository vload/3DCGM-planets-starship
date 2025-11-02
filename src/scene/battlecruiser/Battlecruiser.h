#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <core/config.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <framework/window.h>

struct LightParticle {
    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 color;
    float angle;
    float thresholdLight;
    float intensity;
};

struct BezierCurve {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

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

    std::vector<BezierCurve> getBezierCurves();

    void updatePosition(float deltaTime);

    std::vector<glm::vec3> getRelativePositionThrusters();

    glm::mat4 getModelMatrix();

    glm::vec3 getDirectionVector();

    glm::vec3 getUpVector();

private:
    bool isFollowingPath = true;

    float timeBezierPath = 0;
    int currentCurvePath = -1;
    std::vector<BezierCurve> bezier_curve_list;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));

    std::vector<glm::vec3> relativePositionThrusters = {
        glm::vec3(0.0f, -0.5f, -22.0f),
        glm::vec3(0.0f, 8.5f, -22.0f),
        glm::vec3(4.5f, 4.0f, -22.0f),
        glm::vec3(-4.5f, 4.0f, -22.0f)
    };

    Shader mainShader;
    Shader reflectiveShader;
    

    std::vector<MeshGL> meshGLs;

    void initializeBezierPathMovement();
    void updateVelocityPositionFreeMovement(float deltaTime);
    void updateVelocityPositionPathMovement(float deltaTime);

    static glm::vec3 derivativeBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
    static glm::vec3 bezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
};
