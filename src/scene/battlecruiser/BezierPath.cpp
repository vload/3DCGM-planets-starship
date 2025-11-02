#include "Battlecruiser.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <random>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include "BezierPath.h"

BezierPath::BezierPath() {
    //Create Quad covering the entire screen
    unsigned int quad_indices[6] = {0, 1, 2, 3, 0, 2};

    glm::vec3 quad_vertices[4] = {
        {-1.0f, -1.0f, 0.0f}, // bottom-left
        {1.0f, -1.0f, 0.0f}, // bottom-right
        {1.0f, 1.0f, 0.0f}, // top-right
        {-1.0f, 1.0f, 0.0f} // top-left
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

    // Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);

    glBindVertexArray(0);

    bezierShader =
            ShaderBuilder()
            .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/bezier_vertex.glsl")
            .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/bezier_frag.glsl")
            .build();

    positionPoints = {};
    sizePositionPoints = 0;
}

BezierPath::~BezierPath() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);
}

void BezierPath::draw(const glm::mat4 &view, const glm::mat4 &projection, const glm::mat4 &model, glm::vec2 viewportSize) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    bezierShader.bind();

    glUniform1i(bezierShader.getUniformLocation("numPositions"), sizePositionPoints);
    glUniform3fv(bezierShader.getUniformLocation("listPositions"), (GLsizei) positionPoints.size(),
                 glm::value_ptr(positionPoints[0]));
    glUniform2fv(bezierShader.getUniformLocation("viewportSize"), 1, glm::value_ptr(viewportSize));
    glUniformMatrix4fv(bezierShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(bezierShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(bezierShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::vec3 BezierPath::bezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    return std::pow(1 - t, 3.0f) * p0 + 3 * std::pow(1 - t, 2.0f) * t * p1 + 3 * (1 - t) * std::pow(t, 2.0f) * p2 +
           std::pow(t, 3.0f) * p3;
}

glm::vec3 BezierPath::derivativeBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    return 3 * std::pow(1 - t, 2.0f) * (p1 - p0) + 6 * (1 - t) * t * (p2 - p1) + 3 * std::pow(t, 2.0f) * (p3 - p2);
}

BezierArcLengthTable BezierPath::computeArcLengthTable(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                                                       int samples) {
    BezierArcLengthTable table;
    table.tValues.resize(samples + 1);
    table.accValues.resize(samples + 1);

    table.tValues[0] = 0.0f;
    table.accValues[0] = 0.0f;

    glm::vec3 prev = bezier(p0, p1, p2, p3, 0.0f);
    float sum = 0;

    for (int i = 1; i <= samples; i++) {
        float t = (float) i / (float) samples;
        glm::vec3 pos = bezier(p0, p1, p2, p3, t);
        sum += glm::length(pos - prev);

        table.tValues[i] = t;
        table.accValues[i] = sum;

        prev = pos;
    }

    table.totalLength = sum;
    return table;
}

float BezierPath::findTFromArcLength(const BezierArcLengthTable &table, float distance) {
    if (distance <= 0) return 0.0f;
    if (distance >= table.totalLength) return 1.0f;

    int low = 0;
    int high = table.accValues.size() - 1;

    while (low < high - 1) {
        int mid = (low + high) / 2;
        if (table.accValues[mid] < distance)
            low = mid;
        else
            high = mid;
    }

    float len = table.accValues[high] - table.accValues[low];
    float t = table.tValues[high] - table.tValues[low];
    float ratio = (distance - table.accValues[low]) / len;

    return table.tValues[low] + ratio * t;
}

void BezierPath::initializeBezierPathMovement(const Config::BattlecruiserPathInfo &battlecruiser_path_info,
                                              glm::vec3 current_pos, glm::vec3 current_velocity) {
    // --- Safety checks ---
    const size_t numPoints = battlecruiser_path_info.origin_point_list.size();
    if (numPoints == 0 ||
        battlecruiser_path_info.previous_point_list.size() != numPoints ||
        battlecruiser_path_info.next_point_factor_list.size() != numPoints) {
        std::cerr << "Invalid battlecruiser path configuration! Cannot create Bezier Path";
        return;
    }

    // --- Reset state ---
    timeBezierPath = 0.0f;
    currentCurvePath = 0;
    bezier_curve_list.clear();
    bezier_arc_length_tables_list.clear();
    position = current_pos;
    velocity = current_velocity;

    // --- First curve ---
    glm::vec3 firstControl = position + glm::normalize(velocity) * battlecruiser_path_info.start_point_factor;
    bezier_curve_list.push_back({
        position,
        firstControl,
        battlecruiser_path_info.previous_point_list[0],
        battlecruiser_path_info.origin_point_list[0]
    });

    // --- Middle curves ---
    for (size_t i = 1; i < numPoints; ++i) {
        const glm::vec3 &prevOrigin = battlecruiser_path_info.origin_point_list[i - 1];
        const glm::vec3 &prevPrev = battlecruiser_path_info.previous_point_list[i - 1];
        float nextFactor = battlecruiser_path_info.next_point_factor_list[i - 1];

        glm::vec3 nextOriginPointMiddle = prevOrigin + glm::normalize(prevOrigin - prevPrev) * nextFactor;

        bezier_curve_list.push_back({
            prevOrigin,
            nextOriginPointMiddle,
            battlecruiser_path_info.previous_point_list[i],
            battlecruiser_path_info.origin_point_list[i]
        });
    }

    // --- Last curve ---
    const size_t last = numPoints - 1;
    glm::vec3 lastOrigin = battlecruiser_path_info.origin_point_list[last];
    glm::vec3 lastPrev = battlecruiser_path_info.previous_point_list[last];
    float lastFactor = battlecruiser_path_info.next_point_factor_list[last];

    glm::vec3 nextOriginPointLast = lastOrigin + glm::normalize(lastOrigin - lastPrev) * lastFactor;

    bezier_curve_list.push_back({
        lastOrigin,
        nextOriginPointLast,
        position - glm::normalize(velocity) * battlecruiser_path_info.start_point_factor,
        position
    });

    // --- Compute bezier sum arc-length table for re-parametrization
    for (auto &[p0, p1, p2, p3]: bezier_curve_list) {
        bezier_arc_length_tables_list.push_back(computeArcLengthTable(
            p0,
            p1,
            p2,
            p3
        ));
    }

    const int numSamples = 24;
    positionPoints.clear();
    positionPoints.reserve(bezier_curve_list.size() * numSamples);
    sizePositionPoints = bezier_curve_list.size() * numSamples;
    for (int i = 0; i < bezier_curve_list.size(); i++) {
        BezierCurve controlPointsPath = bezier_curve_list[i];
        BezierArcLengthTable &table = bezier_arc_length_tables_list[i];

        for (int j = 0; j < numSamples; j++) {
            float t = findTFromArcLength(table, (j / float(numSamples - 1)) * table.totalLength);
            positionPoints.push_back(bezier(controlPointsPath.p0, controlPointsPath.p1, controlPointsPath.p2, controlPointsPath.p3, t));
        }
    }
}

void BezierPath::updateVelocityPositionPathMovement(float deltaTime) {
    // if (timeBezierPath == 1.0f) {
    //     timeBezierPath = 0.0f;
    //     currentCurvePath += 1;
    //
    //     if (currentCurvePath == bezier_curve_list.size())
    //         currentCurvePath = 0;
    // }
    //
    // float speed = glm::length(velocity);
    // glm::vec3 derivativeBezier = Battlecruiser::derivativeBezier(bezier_curve_list[currentCurvePath].p0,
    //                                                              bezier_curve_list[currentCurvePath].p1,
    //                                                              bezier_curve_list[currentCurvePath].p2,
    //                                                              bezier_curve_list[currentCurvePath].p3,
    //                                                              timeBezierPath);
    // float lengthCurveBezier = glm::length(derivativeBezier);
    //
    // timeBezierPath += (speed / lengthCurveBezier) * deltaTime;
    // timeBezierPath = glm::clamp(timeBezierPath, 0.0f, 1.0f);
    //
    //
    //
    // glm::vec3 newPosition = bezier(bezier_curve_list[currentCurvePath].p0, bezier_curve_list[currentCurvePath].p1,
    //                   bezier_curve_list[currentCurvePath].p2, bezier_curve_list[currentCurvePath].p3, timeBezierPath);
    //
    // std::cout << timeBezierPath << " " << glm::distance(newPosition, position) << std::endl;
    //
    // position = newPosition;
    // velocity = glm::normalize(derivativeBezier) * speed;
    auto &curve = bezier_curve_list[currentCurvePath];
    auto &table = bezier_arc_length_tables_list[currentCurvePath];

    float speed = glm::length(velocity);
    timeBezierPath += speed * deltaTime;

    if (timeBezierPath >= table.totalLength) {
        // Move to next curve
        timeBezierPath = 0.0f;
        currentCurvePath = (currentCurvePath + 1) % bezier_curve_list.size();

        curve = bezier_curve_list[currentCurvePath];
        table = bezier_arc_length_tables_list[currentCurvePath];
    }

    float t = findTFromArcLength(table, timeBezierPath);
    glm::vec3 newPosition = bezier(curve.p0, curve.p1, curve.p2, curve.p3, t);

    position = newPosition;

    glm::vec3 tangent = derivativeBezier(curve.p0, curve.p1, curve.p2, curve.p3, t);
    velocity = glm::normalize(tangent) * speed;
}

glm::vec3 BezierPath::getCurrentPosition() const {
    return position;
}
glm::vec3 BezierPath::getCurrentVelocity() const {
    return velocity;
}
