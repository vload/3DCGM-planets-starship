// #include "Image.h"

// Always include window first (because it includes glfw, which includes GL
// which needs to be included AFTER glew). Can't wait for modules to fix this
// stuff...
#include <framework/disable_all_warnings.h>
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
#include "scene/bodies/Body.h"
#include "scene/bodies/Earth.h"
#include "scene/bodies/Star.h"
#include "scene/bodies/ico_mesh.h"
#include "scene/camera/Camera.h"
#include "scene/camera/FreeCamera.h"
#include "scene/battlecruiser/Battlecruiser.h"

int WIDTH_WINDOW = 1280;
int HEIGHT_WINDOW = 720;

// Resets OpenGL state to a known default configuration.
void reset_opengl_state() {
    // Viewport
    glViewport(0, 0, WIDTH_WINDOW, HEIGHT_WINDOW);

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

    // Framebuffer bindings
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void load_config(Config& config) {
    try {
        config.load_config(RESOURCE_ROOT "settings.toml");
    } catch (const std::exception& e) {
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
    // TODO: set up battlecruiser camera (it needs another camera class
    // probably)
    //  - use the battlecruiser's position as lookat target
    //  - position move with mouse on a sphere around the battlecruiser
    //  (!tricky)
    // BattlecruiserCamera battlecruiserCam(window, config, battlecruiser);
    // TODO: minimap camera

    // TODO: toggle camera button from battlecruiser to freecam and back
    Camera* active_camera = &freecam;
    // Camera *minimap_camera = &minimapCam;

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
    // TODO: we might need other projection matrices for other cameras (minimap, shadows)

    /// ---- Scene setup
    /// -- Planets
    GPUMesh ico_mesh{Mesh{}};
    try {
        Mesh ico_mesh_cpu =
            generate_ico_mesh(config.planets_ico_mesh_resolution);
        ico_mesh = GPUMesh(ico_mesh_cpu);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create icosahedron GPUMesh: " << e.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<Body*> bodies;
    int selected_body = 0;
    // TODO: load bodies from config
    bodies.push_back(
        new Star(config, glm::vec3(0.0f, 0.0f, 0.0f), 4.0f, ico_mesh));
    bodies.push_back(
        new Earth(config, glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, ico_mesh));
    bodies[1]->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 20.0f, 30.0f,
                         glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, bodies[0]);
    bodies.push_back(
        new Body(config, glm::vec3(0.0f, 0.0f, 0.0f), 0.4f, ico_mesh));
    bodies[2]->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 6.0f, 9.0f,
                         glm::vec3(0.0f, 1.0f, 0.0f), 3.0f, bodies[1]);

    for (auto& body : bodies) {
        body->setup();
    }

    /// -- Skybox
    Skybox skybox;

    /// -- Battlecruiser
    Battlecruiser battlecruiser;

    /// -- Battlecruiser Particles
    ParticleSystem particles;
    // TODO: move these to particle system class
    // TODO: load particle system params from config
    glm::vec3 speedInitParticle = glm::vec3(0.0f, 0.0f, 2.5f);
    glm::vec3 speedParticle = glm::vec3(0.0f, 0.0f, -8.5f);
    glm::vec2 colorR = glm::vec2(233.0f, 255.f);
    glm::vec2 colorG = glm::vec2(165.0f, 255.f);
    glm::vec2 colorB = glm::vec2(0.0f, 0.0f);
    float coneAngle = 50.0f;
    float life = 2.0f;
    float lifeDeviation = 2.0f;
    float lifeThreshold = 1.0f;
    float size = 1.0f;
    float sizeDeviation = 1.0f;

    /// ---- Other setup
    // TODO : ????

    /// -- Timing
    static double last_time = glfwGetTime();
    /// -- Debug mode
    static bool debug_mode = false;

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
        /// -- Update bodies
        for (Body* body : bodies) {
            body->update((float) delta_time);
        }

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
        } else {  // not debug mode
            /// -- ImGui Body selection and controls
            ImGui::Separator();
            ImGui::SliderInt("Selected Body", &selected_body, 0,
                             (int)bodies.size() - 1);
            bodies[selected_body]->imGuiControl();

            // TODO: move these to particle system
            /// -- ImGui Particle System Controls
            ImGui::Separator();
            // --- Particle Speed ---
            ImGui::Text("Particle Speed");
            ImGui::DragFloat3("Speed Init (x, y, z)",
                              glm::value_ptr(speedInitParticle), 0.1f, -10.0f,
                              10.0f);
            ImGui::DragFloat3("Speed (x, y, z)", glm::value_ptr(speedParticle),
                              0.1f, -10.0f, 10.0f);

            // --- Color Ranges ---
            ImGui::Separator();
            ImGui::Text("Color Range (RGB Min/Max)");
            ImGui::DragFloat2("Red Range", glm::value_ptr(colorR), 1.0f, 0.0f,
                              255.0f);
            ImGui::DragFloat2("Green Range", glm::value_ptr(colorG), 1.0f, 0.0f,
                              255.0f);
            ImGui::DragFloat2("Blue Range", glm::value_ptr(colorB), 1.0f, 0.0f,
                              255.0f);

            // --- Cone Angle ---
            ImGui::Separator();
            ImGui::Text("Emission Cone");
            ImGui::SliderFloat("Cone Angle", &coneAngle, 0.0f, 180.0f,
                               "%.1f deg");

            // --- Life & Size ---
            ImGui::Separator();
            ImGui::Text("Particle Properties");
            ImGui::DragFloat("Life", &life, 0.1f, 0.0f, 10.0f);
            ImGui::DragFloat("Life Deviation", &lifeDeviation, 0.1f, 0.0f,
                             10.0f);
            ImGui::DragFloat("Life Threshold", &lifeThreshold, 0.1f, 0.1f,
                             10.0f);

            ImGui::DragFloat("Size", &size, 0.1f, 0.0f, 10.0f);
            ImGui::DragFloat("Size Deviation", &sizeDeviation, 0.1f, 0.0f,
                             10.0f);
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

            /// -- Pass #2: Render Bodies
            reset_opengl_state();
            for (auto& body : bodies) {
                body->shader.bind();
                glUniform1f(body->shader.getUniformLocation("screenHeight"),
                            (float)window.getWindowSize().y);
                glUniform1f(body->shader.getUniformLocation("fov"),
                            glm::radians(config.camera_fov_degrees));
                glUniform1f(body->shader.getUniformLocation("time"),
                            (float)glfwGetTime());

                body->draw(active_camera->get_view_matrix(), projection_matrix,
                           active_camera->get_position());
            }

            /// -- Pass #3 and #4: Render Battlecruiser Mesh
            // TODO: separate these two passes if needed (for different shaders)
            reset_opengl_state();
            battlecruiser.draw(active_camera->get_view_matrix(),
                projection_matrix, glm::vec3(10.0f, 10.0f, 10.0f), active_camera->get_position(),
                skybox.getCubemapTexture());
            
            /// -- Pass #5: Render battlecruiser Particles
            // TODO: these should be in the particle class.
            reset_opengl_state();
            float particlesPerSecond = 1000.0f;
            int newParticles = static_cast<int>(delta_time * particlesPerSecond);
            if (newParticles > 500)
                newParticles = 500;

            for (int i = 0; i < newParticles; ++i) {
                for (int j = 0; j < battlecruiser.getRelativePositionThrusters().size(); j++) {
                    particles.spawn(battlecruiser.getRelativePositionThrusters()[j], speedInitParticle, colorR, colorG, colorB, coneAngle, life, lifeDeviation, size, sizeDeviation);
                }
            }

            particles.update(battlecruiser.getModelMatrix(), (float) delta_time, active_camera->get_position(), speedParticle, lifeThreshold);
            particles.draw(active_camera->get_view_matrix(), projection_matrix, battlecruiser.getModelMatrix());
        }

        //// ---- Swap buffers
        window.swapBuffers();
    }

    // cleanup and exit
    return 0;
}