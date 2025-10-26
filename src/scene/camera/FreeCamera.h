// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <framework/window.h>

#include "core/config.h"
#include "scene/camera/Camera.h"


class FreeCamera: public Camera {
    Window& window;
    Config& config;

    glm::vec3 position { 0 };
    glm::vec3 forward { 0, 0, -1 };
    glm::vec3 up { 0, 1, 0 };

    bool user_interaction { true };
    glm::dvec2 previous_cursor_position { 0 };
    
    const float move_speed;
    const float look_speed;

    static constexpr glm::vec3 Y_axis{0, 1, 0};

public:
    FreeCamera(Window& window, Config& config): 
        window(window), 
        config(config), 
        move_speed(config.freecam_move_speed),
        look_speed(config.freecam_look_speed)
    {
        position = config.freecam_initial_position;
        forward = glm::normalize(config.freecam_initial_forward);
    }

    void update_input(){
        if(!user_interaction){
            previous_cursor_position = window.getCursorPos();
            return;
        } 


        // movement
        glm::vec3 localMoveDelta { 0 };
        const glm::vec3 right = glm::normalize(glm::cross(forward, up));
        if (window.isKeyPressed(GLFW_KEY_A))
            position -= move_speed * right;
        if (window.isKeyPressed(GLFW_KEY_D))
            position += move_speed * right;
        if (window.isKeyPressed(GLFW_KEY_W))
            position += move_speed * forward;
        if (window.isKeyPressed(GLFW_KEY_S))
            position -= move_speed * forward;
        if (window.isKeyPressed(GLFW_KEY_R))
            position += move_speed * up;
        if (window.isKeyPressed(GLFW_KEY_F))
            position -= move_speed * up;

        // looking around
        const glm::dvec2 cursor_position = window.getCursorPos();
        const glm::vec2 delta = look_speed * glm::vec2(cursor_position - previous_cursor_position);
        previous_cursor_position = cursor_position;

        if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (delta.x != 0.0f)
                rotate_Y(delta.x);
            if (delta.y != 0.0f)
                rotate_X(delta.y);
        }
    }
 
    void set_user_interaction(bool enabled) {
        user_interaction = enabled;
    }

    glm::vec3 get_position() const {
        return position;
    }

    glm::mat4 get_view_matrix() const{
        return glm::lookAt(position, position + forward, up);
    }

private:
    void rotate_X(float angle) {
        const glm::vec3 horizontal_axis = glm::cross(Y_axis, forward);

        forward = glm::normalize(glm::angleAxis(angle, horizontal_axis) * forward);
        up = glm::normalize(glm::cross(forward, horizontal_axis));
    }

    void rotate_Y(float angle) {
        const glm::vec3 horizontal_axis = glm::cross(Y_axis, forward);

        forward = glm::normalize(glm::angleAxis(angle, Y_axis) * forward);
        up = glm::normalize(glm::cross(forward, horizontal_axis));
    }
};
