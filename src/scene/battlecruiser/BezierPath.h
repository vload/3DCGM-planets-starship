#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <core/config.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <framework/window.h>
#include "Battlecruiser.h"

class BezierPath {
    Battlecruiser& battlecruiser;
public:
    BezierPath(Battlecruiser& battlecruiser);
    ~BezierPath();

    void draw(const glm::mat4& view,
        const glm::mat4& projection);

private:
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    Shader bezierShader;
};