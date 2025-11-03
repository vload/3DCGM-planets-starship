#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <core/config.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <framework/window.h>
#include "Battlecruiser.h"

struct BezierArcLengthTable {
    std::vector<float> tValues;
    std::vector<float> accValues;
    float totalLength;
};

struct BezierCurve {
    glm::vec3 p0;
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
};

class BezierPath {
public:
    BezierPath();
    ~BezierPath();

    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, glm::vec2 viewportSize);
    void initializeBezierPathMovement(const Config::BattlecruiserPathInfo & battlecruiser_path_info, glm::vec3 current_pos, glm::vec3 current_velocity );
    void updateVelocityPositionPathMovement(float deltaTime);

    glm::vec3 getCurrentPosition() const;
    glm::vec3 getCurrentVelocity() const;

private:
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    Shader bezierShader;

    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 1.0f);

    std::vector<glm::vec3> positionPoints;
    int sizePositionPoints;

    float timeBezierPath = 0;
    int currentCurvePath = -1;
    std::vector<BezierCurve> bezier_curve_list;

    std::vector<BezierArcLengthTable> bezier_arc_length_tables_list;

    static float findTFromArcLength(const BezierArcLengthTable& table, float distance);
    static glm::vec3 bezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
    static glm::vec3 derivativeBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
    static BezierArcLengthTable computeArcLengthTable(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int samples = 200);
};