#pragma once
#include <framework/disable_all_warnings.h>
#include <framework/shader.h>

DISABLE_WARNINGS_PUSH()
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <framework/window.h>

#include "core/config.h"
#include "scene/camera/Camera.h"


class BattlecruiserCamera : public Camera {
    Window& window;
    Config& config;
    Battlecruiser& battlecruiser;

    glm::vec3 position{ 0 };
    glm::vec3 forward{ 0, 0, -1 };
    glm::vec3 up{ 0, 1, 0 };
    float offset = 5.0f;

    bool user_interaction{ true };
    glm::dvec2 previous_cursor_position{ 0 };

    const float look_speed;

    static constexpr glm::vec3 Y_axis{ 0, 1, 0 };

public:
    BattlecruiserCamera(Window& window, Config& config, Battlecruiser& battlecruiser):
        window(window),
        config(config),
        battlecruiser(battlecruiser),
        look_speed(config.freecam_look_speed)
    {
        glm::vec3 target = glm::vec3(battlecruiser.getModelMatrix()[3]);
        glm::vec3 dir = battlecruiser.getDirectionVector();

        position = target - dir * offset;
        forward = dir;

        const glm::vec3 horizontal_axis = glm::cross(Y_axis, forward);
        up = glm::normalize(glm::cross(forward, horizontal_axis));
    }

    void update_input() {
        if (!user_interaction) {
            previous_cursor_position = window.getCursorPos();
            return;
        }

        glm::vec3 target = glm::vec3(battlecruiser.getModelMatrix()[3]);
        position = target - forward * offset;

        // looking around
        const glm::dvec2 cursor_position = window.getCursorPos();
        const glm::vec2 delta = look_speed * glm::vec2(cursor_position - previous_cursor_position);
        previous_cursor_position = cursor_position;

        if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (delta.x != 0.0f)
                rotate_Y_around(delta.x);
            if (delta.y != 0.0f)
                rotate_X_around(delta.y);
        } 
    }

    void set_user_interaction(bool enabled) {
        user_interaction = enabled;
    }

    glm::vec3 get_position() const {
        return position;
    }

    glm::mat4 get_view_matrix() const {
        return glm::lookAt(position, position + forward, up);
    }

private:
    void rotate_X_around(float angle) {
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        glm::vec3 target = glm::vec3(battlecruiser.getModelMatrix()[3]);
        glm::vec3 targerDir = position - target;
        float currentPitch = glm::degrees(asin(forward.y));
        float maxPitch = 89.0f;

        float newPitch = glm::clamp(currentPitch + glm::degrees(angle), -maxPitch, maxPitch);
        float clampedAngle = glm::radians(newPitch - currentPitch);

        targerDir = glm::angleAxis(clampedAngle, right) * targerDir;

        position = target + targerDir;
        forward = glm::normalize(target - position);
    }

    void rotate_Y_around(float angle) {
        glm::vec3 target = glm::vec3(battlecruiser.getModelMatrix()[3]);
        glm::vec3 targerDir = position - target;
        targerDir = glm::angleAxis(-angle, glm::normalize(glm::vec3(glm::mat3(battlecruiser.getModelMatrix()) * Y_axis))) * targerDir;

        position = target + targerDir;

        forward = glm::normalize(target - position);
    }
};
