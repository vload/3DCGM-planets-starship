#pragma once
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

class Camera {
public:
    virtual ~Camera() = default;
    virtual glm::vec3 get_position() const = 0;
    virtual glm::mat4 get_view_matrix() const = 0;
};