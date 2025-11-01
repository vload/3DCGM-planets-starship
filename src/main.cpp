// #include "Image.h"

// Always include window first (because it includes glfw, which includes GL
// which needs to be included AFTER glew). Can't wait for modules to fix this
// stuff...
#include <framework/disable_all_warnings.h>
#include <scene/battlecruiser/ParticleSystem.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
// Include glad before glfw3
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/shader.h>
#include <framework/window.h>

#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

#include "core/config.h"
#include "core/mesh.h"
#include "scene/Skybox.h"
#include "scene/battlecruiser/Battlecruiser.h"
#include "scene/bodies/PlanetSystem.h"
#include "scene/camera/Camera.h"
#include "scene/camera/FreeCamera.h"
#include "scene/camera/BattlecruiserCamera.h"

int WIDTH_WINDOW = 1280;
int HEIGHT_WINDOW = 720;

// Resets OpenGL state to a known default configuration.
void reset_opengl_state() {
    // Viewport
    glViewport(0, 0, WIDTH_WINDOW, HEIGHT_WINDOW);

    // Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Depth test
    glClearDepth(1.0f);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Tessellation
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void load_config(Config &config) {
    try {
        config.load_config(RESOURCE_ROOT "settings.toml");
    } catch (const std::exception &e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // Handle error appropriately (e.g., set default values or exit)
        // For now, we'll just exit
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    //// -------- Setup:
    /// ---- Load configuration
    Config config;
    load_config(config);

    /// ---- Window setup
    WIDTH_WINDOW = config.window_initial_width;
    HEIGHT_WINDOW = config.window_initial_height;

    Window window(config.window_title, glm::ivec2(WIDTH_WINDOW, HEIGHT_WINDOW),
                  OpenGLVersion::GL41);
    bool window_resized = false;
    window.registerWindowResizeCallback([&](glm::ivec2 newSize) {
        std::cout << "Window resized to: " << newSize.x << "x" << newSize.y
                << std::endl;
        window_resized = true;
    });
    window.registerWindowResizeCallback([&](glm::ivec2 newSize) {
        // Update width and height globals on resize
        WIDTH_WINDOW = newSize.x;
        HEIGHT_WINDOW = newSize.y;
    });

    /// ---- Camera setup
    FreeCamera freecam(window, config);

    Camera *active_camera = &freecam;

    // TODO: minimap camera

    glm::mat4 projection_matrix = glm::perspective(
        glm::radians(config.camera_fov_degrees),
        static_cast<float>(WIDTH_WINDOW) / static_cast<float>(HEIGHT_WINDOW),
        0.1f, 1000.0f);
    window.registerWindowResizeCallback([&](glm::ivec2 newSize) {
        projection_matrix = glm::perspective(
            glm::radians(config.camera_fov_degrees),
            static_cast<float>(newSize.x) / static_cast<float>(newSize.y), 0.1f,
            1000.0f);
    });
    // TODO: we might need other projection matrices for other cameras (minimap)

    /// ---- Scene setup
    /// -- Planets
    PlanetSystem planet_system(config);

    /// -- Skybox
    Skybox skybox;

    /// -- Battlecruiser
    Battlecruiser battlecruiser(window);

    /// -- Camera Battlecruiser
    BattlecruiserCamera battlecruiserCamera(window, config, battlecruiser);

    /// -- Battlecruiser Particles
    ParticleSystem particles(battlecruiser);

    window.registerKeyCallback([&](int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_1) {
                active_camera = &freecam;
            }
            if (key == GLFW_KEY_2) {
                active_camera = &battlecruiserCamera;
            }
        }
    });

    /// ---- Other setup
    // TODO : ????

    /// -- Timing
    double last_time = glfwGetTime();
    float time_warp = 1.0f;
    /// -- Debug mode
    bool debug_mode = false;

    /// -------- Main loop:
    while (!window.shouldClose()) {
        /// ---- Updates
        /// -- Update timing
        double delta_time = glfwGetTime() - last_time;
        last_time = glfwGetTime();
        /// -- Update inputs
        window.updateInput();
        /// -- Update cameras
        freecam.update_input();
        // -- Update battlecruiser
        battlecruiser.updateVelocityPosition(static_cast<float>(delta_time));
        // -- Update battlecruiser camera
        battlecruiserCamera.update_input();
        // -- Update battlecruiser particles
        particles.update(active_camera->get_position(), static_cast<float>(delta_time));

        /// -- Update bodies
        planet_system.update(time_warp * (float) delta_time);

        /// ---- ImGui
        // window.update_input already called ImGui::NewFrame()
        ImGui::Begin("Window");
        /// -- ImGui debug mode
        ImGui::Checkbox("Debug Mode", &debug_mode);
        /// -- Reload config button
        if (ImGui::Button("Reload Config")) {
            load_config(config);
            std::cout << "Config reloaded." << std::endl;
        }
        if (debug_mode) {
            // nothing to debug
        } else {
            // not debug mode
            ImGui::SliderFloat("Time Warp", &time_warp, 0.0f, 10.0f, "%.2f x");
            /// -- ImGui Body selection and controls
            planet_system.imgui();

            ImGui::Separator();
        }

        ImGui::End();

        //// ---- Rendering

        /// -- Set states and clear buffers
        reset_opengl_state();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (debug_mode) {
        } else {
            /// -- Pass #1: Render Skybox
            reset_opengl_state();
            skybox.draw(active_camera->get_view_matrix(), projection_matrix);

            /// -- Pass #2 and #3: Shadow-map per body(that needs it) and Render Bodies
            reset_opengl_state();
            planet_system.draw(active_camera->get_view_matrix(),
                               projection_matrix, active_camera->get_position(),
                               static_cast<float>(HEIGHT_WINDOW),
                               reset_opengl_state);

            /// -- Pass #4 and #5: Render Battlecruiser Mesh
            reset_opengl_state();
            battlecruiser.draw(
                active_camera->get_view_matrix(), projection_matrix,
                glm::vec3(10.0f, 10.0f, 10.0f), active_camera->get_position(),
                skybox.getCubemapTexture());

            /// -- Pass #6: Render battlecruiser Particles
            reset_opengl_state();
            particles.draw_stage(active_camera->get_view_matrix(), projection_matrix);
        }

        //// ---- Swap buffers
        window.swapBuffers();
    }

    // cleanup and exit
    return 0;
}
