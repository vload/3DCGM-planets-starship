// #include "Image.h"
#include "Body.h"
#include "mesh.h"
#include "texture.h"
#include "camera.h"
#include "Star.h"
#include "Earth.h"
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
#include "utils/Battlecruiser.h"
#include "utils/SkyboxSystem.h"

// Forward declaration for the helper defined below
static Mesh makeGeodesicIcosahedronMesh(int frequency);

class Application {
   public:
    Application()
        : m_window("Final Project", glm::ivec2(WIDTH_WINDOW, HEIGHT_WINDOW),
                   OpenGLVersion::GL41),
          m_texture(RESOURCE_ROOT "resources/checkerboard.png"),
          m_camera(&m_window) {
        m_window.registerKeyCallback(
            [this](int key, int scancode, int action, int mods) {
                if (action == GLFW_PRESS)
                    onKeyPressed(key, mods);
                else if (action == GLFW_RELEASE)
                    onKeyReleased(key, mods);
            });
        m_window.registerMouseMoveCallback(
            std::bind(&Application::onMouseMove, this, std::placeholders::_1));
        m_window.registerMouseButtonCallback(
            [this](int button, int action, int mods) {
                if (action == GLFW_PRESS)
                    onMouseClicked(button, mods);
                else if (action == GLFW_RELEASE)
                    onMouseReleased(button, mods);
            });
        m_window.registerScrollCallback(
            [this](const glm::vec2& offset) { onMouseScroll(offset); });
        m_window.registerWindowResizeCallback(
            [this](glm::ivec2 newSize) {
                std::cout << "Window resized to: " << newSize.x << "x" << newSize.y << std::endl;

                glm::ivec2 frameBufferSize = m_window.getFrameBufferSize();

                aspect_window = static_cast<float>(frameBufferSize.x) / frameBufferSize.y;
                width_frame_window = frameBufferSize.x;
                height_frame_window = frameBufferSize.y;

                m_projectionMatrix = glm::perspective(fov_radians, aspect_window, 0.1f, 1000.0f);
            }
        );

        glm::ivec2 frameBufferSize = m_window.getFrameBufferSize();

        aspect_window = static_cast<float>(frameBufferSize.x) / frameBufferSize.y;
        width_frame_window = frameBufferSize.x;
        height_frame_window = frameBufferSize.y;

        m_projectionMatrix = glm::perspective(fov_radians, aspect_window, 0.1f, 1000.0f);

        // Load model meshes (from file) and also create a hardcoded icosahedron
        // mesh
        m_modelMeshes =
            GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/dragon.obj");

        // Create a hardcoded icosahedron CPU mesh and upload to GPU
        Mesh ico = makeGeodesicIcosahedronMesh(5);
        try {
            m_icosaMesh = GPUMesh(ico);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create icosahedron GPUMesh: " << e.what()
                      << std::endl;
        }

        m_bodies.push_back(
            new Star(glm::vec3(0.0f, 0.0f, 0.0f), 4.0f, m_icosaMesh));
        m_bodies.push_back(
            new Earth(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, m_icosaMesh));
        m_bodies[1]->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 20.0f, 30.0f,
                              glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, m_bodies[0]);
        m_bodies.push_back(new Body(glm::vec3(0.0f, 0.0f, 0.0f), 0.4f, m_icosaMesh));
        m_bodies[2]->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 6.0f, 9.0f,
                              glm::vec3(0.0f, 1.0f, 0.0f), 3.0f, m_bodies[1]);

        for (auto& body : m_bodies) {
            body->setup();
        }

        try {
            ShaderBuilder defaultBuilder;
            defaultBuilder.addStage(GL_VERTEX_SHADER,
                                    RESOURCE_ROOT "shaders/shader_vert.glsl");
            defaultBuilder.addStage(GL_FRAGMENT_SHADER,
                                    RESOURCE_ROOT "shaders/shader_frag.glsl");
            m_defaultShader = defaultBuilder.build();

            ShaderBuilder shadowBuilder;
            shadowBuilder.addStage(GL_VERTEX_SHADER,
                                   RESOURCE_ROOT "shaders/shadow_vert.glsl");
            shadowBuilder.addStage(GL_FRAGMENT_SHADER,
                                   RESOURCE_ROOT "Shaders/shadow_frag.glsl");
            m_shadowShader = shadowBuilder.build();

            ShaderBuilder icoBuilder;
            icoBuilder.addStage(GL_VERTEX_SHADER,
                                RESOURCE_ROOT "shaders/ico_vert.glsl");
            icoBuilder.addStage(GL_TESS_CONTROL_SHADER,
                                RESOURCE_ROOT "shaders/ico_tesc.glsl");
            icoBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                                RESOURCE_ROOT "shaders/ico_tese.glsl");
            icoBuilder.addStage(GL_FRAGMENT_SHADER,
                                RESOURCE_ROOT "shaders/ico_frag.glsl");
            m_icoShader = icoBuilder.build();

            battlecruiserShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/battlecruiser/shader_vert.glsl").addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/battlecruiser/shader_frag.glsl").build();
            battlecruiserReflectiveShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/battlecruiser/shader_vert.glsl").addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/battlecruiser/glass_shader_frag.glsl").build();
            particleShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/battlecruiser/particle_vertex.glsl").addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/battlecruiser/particle_frag.glsl").build();
            skyboxShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/battlecruiser/skybox_vert.glsl").addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/battlecruiser/skybox_frag.glsl").build();

            // Any new shaders can be added below in similar fashion.
            // ==> Don't forget to reconfigure CMake when you do!
            //     Visual Studio: PROJECT => Generate Cache for ComputerGraphics
            //     VS Code: ctrl + shift + p => CMake: Configure => enter
            // ....
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void update() {
        int dummyInteger = 0;  // Initialized to 0
        int selected_body = 0;
        static double lastTime = glfwGetTime();

        while (!m_window.shouldClose()) {
            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();
            m_camera.updateInput();

            // Update camera position
            double deltaTime = glfwGetTime() - lastTime;
            lastTime = glfwGetTime();
            for (auto& body : m_bodies) {
                body->update((float)deltaTime);
            }

            // Use ImGui for easy input/output of ints, floats, strings, etc...
            ImGui::Begin("Window");

            if (m_renderMode == 0) {
                // --- Debug Mode GUI settings ---
 
                // Render mode combo: 0 = loaded model (default), 1 = icosahedron
                static const char* meshItems[] = { "Loaded model", "Icosahedron" };
                ImGui::Combo("Render Mode", &m_renderMode, meshItems,
                    IM_ARRAYSIZE(meshItems));
            }
            else {
                // --- Planets Mode GUI settings ---
                ImGui::InputInt(
                    "This is an integer input",
                    &dummyInteger);  // Use ImGui::DragInt or ImGui::DragFloat for
                // larger range of numbers.
                ImGui::Text("Value is: %i",
                    dummyInteger);  // Use C printf formatting rules (%i is
                // a signed integer)
                ImGui::Checkbox("Use material if no texture", &m_useMaterial);
                ImGui::Checkbox("Wireframe", &m_wireframe);
                ImGui::Checkbox("Body Tessellation", &m_bodyTessellation);
                ImGui::SliderFloat("Target Tessellation Triangle Height",
                    &target_tessellation_triangle_height, 1.0f,
                    20.0f);

                // Render mode combo: 0 = loaded model (default), 1 = icosahedron
                static const char* meshItems[] = { "Loaded model", "Icosahedron" };
                ImGui::Combo("Render Mode", &m_renderMode, meshItems,
                    IM_ARRAYSIZE(meshItems));

                ImGui::Separator();
                ImGui::SliderInt("Selected Body", &selected_body, 0,
                    (int)m_bodies.size() - 1);
                m_bodies[selected_body]->imGuiControl();

                ImGui::Separator();

                // --- Particle Speed ---
                ImGui::Text("Particle Speed");
                ImGui::DragFloat3("Speed Init (x, y, z)", glm::value_ptr(speedInitParticle), 0.1f, -10.0f, 10.0f);
                ImGui::DragFloat3("Speed (x, y, z)", glm::value_ptr(speedParticle), 0.1f, -10.0f, 10.0f);

                // --- Color Ranges ---
                ImGui::Separator();
                ImGui::Text("Color Range (RGB Min/Max)");
                ImGui::DragFloat2("Red Range", glm::value_ptr(colorR), 1.0f, 0.0f, 255.0f);
                ImGui::DragFloat2("Green Range", glm::value_ptr(colorG), 1.0f, 0.0f, 255.0f);
                ImGui::DragFloat2("Blue Range", glm::value_ptr(colorB), 1.0f, 0.0f, 255.0f);

                // --- Cone Angle ---
                ImGui::Separator();
                ImGui::Text("Emission Cone");
                ImGui::SliderFloat("Cone Angle", &coneAngle, 0.0f, 180.0f, "%.1f deg");

                // --- Life & Size ---
                ImGui::Separator();
                ImGui::Text("Particle Properties");
                ImGui::DragFloat("Life", &life, 0.1f, 0.0f, 10.0f);
                ImGui::DragFloat("Life Deviation", &lifeDeviation, 0.1f, 0.0f, 10.0f);
                ImGui::DragFloat("Life Threshold", &lifeThreshold, 0.1f, 0.1f, 10.0f);

                ImGui::DragFloat("Size", &size, 0.1f, 0.0f, 10.0f);
                ImGui::DragFloat("Size Deviation", &sizeDeviation, 0.1f, 0.0f, 10.0f);
            }

            ImGui::End();

            renderFrame(deltaTime);

            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__keys.html mods - Any modifier
    // keys pressed, like shift or control
    void onKeyPressed(int key, int mods) {
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__keys.html mods - Any modifier
    // keys pressed, like shift or control
    void onKeyReleased(int key, int mods) {
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y
    // screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos) {
        std::cout << "Mouse moved: " << cursorPos.x << " " << cursorPos.y
                  << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__buttons.html mods - Any modifier
    // buttons pressed
    void onMouseClicked(int button, int mods) {
        std::cout << "Mouse button pressed: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__buttons.html mods - Any modifier
    // buttons pressed
    void onMouseReleased(int button, int mods) {
        std::cout << "Mouse button released: " << button << std::endl;
    }

    // Handle mouse scroll (e.g., for zooming)
    // offset.x - horizontal scroll, offset.y - vertical scroll (typically used
    // for zoom)
    void onMouseScroll(const glm::vec2& offset) {
        std::cout << "Mouse scroll: " << offset.x << " " << offset.y
                  << std::endl;
    }

    // Render the current frame (clears, sets states, and draws selected meshes)
    void renderFrame(double deltaTime) {
        // Simple dispatcher: call the appropriate full-mode renderer
        if (m_renderMode == 0) {
            renderDefault();
        } else {
            renderIco(deltaTime);
        }
    }

    // Full default renderer: clears and renders all loaded model meshes
    // (matches original behavior)
    void renderDefault() {
        glViewport(0, 0, width_frame_window, height_frame_window);
        glClearDepth(1.0);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    // Full icosahedron renderer: clears and renders the procedural icosahedron
    // meshes (wireframe respected)
    void renderIco(double deltaTime) {
        // Clear the screen (keep same background as default)
        glViewport(0, 0, width_frame_window, height_frame_window);
        glClearDepth(1.0);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glPatchParameteri(GL_PATCH_VERTICES, 3);

        // -- Pass skybox ---
        skyboxSystem.draw(m_camera.viewMatrix(), m_projectionMatrix, skyboxShader);
        
        glClear(GL_DEPTH_BUFFER_BIT);

        // Respect wireframe toggle
        if (m_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        for (auto& body : m_bodies) {
            glm::mat4 modelMatrix = body->get_model_matrix();
            glm::mat4 viewMatrix = m_camera.viewMatrix();

            const glm::mat4 mvpMatrix =
                m_projectionMatrix * viewMatrix * modelMatrix;

            body->shader.bind();
            glUniformMatrix4fv(body->shader.getUniformLocation("mvpMatrix"), 1,
                               GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniform3fv(body->shader.getUniformLocation("cameraWorldPos"), 1,
                         glm::value_ptr(m_camera.cameraPos()));
            glUniform1i(body->shader.getUniformLocation("tessellate"),
                        m_bodyTessellation ? 1 : 0);
            glUniform1f(body->shader.getUniformLocation("screenHeight"),
                        (float)m_window.getWindowSize().y);
            glUniform1f(body->shader.getUniformLocation("fov"),
                        fov_radians);
            glUniform1f(body->shader.getUniformLocation("time"),
                        (float)glfwGetTime());
            glUniform1f(body->shader.getUniformLocation("targetPixelSize"),
                        target_tessellation_triangle_height);

            body->draw();
        }

        {
            // --- Pass: battlecruiser mesh ---
            battlecruiser.draw(battlecruiser.getModelMatrix(), m_camera.viewMatrix(), m_projectionMatrix, glm::vec3(0.0f), m_camera.cameraPos(), battlecruiserShader, battlecruiserReflectiveShader, skyboxSystem.getCubemapTexture());

            // --- Pass: battlecruiser Particles ---
            float particlesPerSecond = 1000.0f;
            int newParticles = static_cast<int>(deltaTime * particlesPerSecond);
            if (newParticles > 500)
                newParticles = 500;

            for (int i = 0; i < newParticles; ++i) {
                for (int j = 0; j < battlecruiser.getRelativePositionThrusters().size(); j++) {
                    particles.spawn(battlecruiser.getRelativePositionThrusters()[j], speedInitParticle, colorR, colorG, colorB, coneAngle, life, lifeDeviation, size, sizeDeviation);
                }
            }

            particles.update(battlecruiser.getModelMatrix(), deltaTime, m_camera.cameraPos(), speedParticle, lifeThreshold);
            particles.draw(m_camera.viewMatrix(), m_projectionMatrix, battlecruiser.getModelMatrix(), particleShader);
        }
    }

   private:
    const int WIDTH_WINDOW = 1080;
    const int HEIGHT_WINDOW = 1080;

    float aspect_window = 1.0f;
    int width_frame_window = 0;
    int height_frame_window = 0;

    Window m_window;

    Camera m_camera;

    // Shader for default rendering and for depth rendering
    Shader m_defaultShader;
    Shader m_shadowShader;
    // Shader with tessellation for rendering the bodies
    Shader m_icoShader;

    Shader battlecruiserShader;
    Shader battlecruiserReflectiveShader;
    Shader particleShader;
    Shader skyboxShader;

    Battlecruiser battlecruiser;
    SkyboxSystem skyboxSystem;
    ParticleSystem particles;

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

    // Keep separate collections for model meshes and the procedurally created
    // icosahedron
    std::vector<GPUMesh> m_modelMeshes;
    GPUMesh m_icosaMesh{Mesh{}};
    int m_renderMode{1};
    Texture m_texture;
    bool m_useMaterial{true};
    bool m_wireframe{false};
    bool m_bodyTessellation{true};

    std::vector<Body *> m_bodies;

    // Tessellation control
    float target_tessellation_triangle_height = 5.0f;

    float fov_radians = glm::radians(80.0f);
    glm::mat4 m_projectionMatrix =
        glm::perspective(fov_radians, aspect_window, 0.1f, 1000.0f);
    glm::mat4 m_modelMatrix{1.0f};
};

// Helper: create a CPU-side Mesh that represents a class I geodesic icosahedron
// centered at origin, with given frequency (>=1).
static Mesh makeGeodesicIcosahedronMesh(int frequency) {
    Mesh mesh;
    if (frequency < 1) frequency = 1;

    const float t = (1.0f + sqrtf(5.0f)) * 0.5f;

    // Base icosahedron vertices
    std::vector<glm::vec3> baseVerts = {{-1, t, 0},  {1, t, 0},   {-1, -t, 0},
                                        {1, -t, 0},  {0, -1, t},  {0, 1, t},
                                        {0, -1, -t}, {0, 1, -t},  {t, 0, -1},
                                        {t, 0, 1},   {-t, 0, -1}, {-t, 0, 1}};
    for (auto& v : baseVerts) v = glm::normalize(v);

    // Base icosahedron faces
    const std::vector<glm::uvec3> baseFaces = {
        {0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
        {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
        {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1}};

    // const std::vector<glm::uvec3> baseFaces = {{0, 11, 5}};

    // Helper: interpolate between two vertices
    auto interpolate = [&](const glm::vec3& a, const glm::vec3& b, int i,
                           int n) {
        return glm::normalize(glm::mix(a, b, float(i) / float(n)));
    };

    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> tris;

    // Subdivide each face
    for (const auto& face : baseFaces) {
        glm::vec3 v0 = baseVerts[face.x];
        glm::vec3 v1 = baseVerts[face.y];
        glm::vec3 v2 = baseVerts[face.z];

        // Generate subdivided points on the face
        std::vector<std::vector<int>> grid(frequency + 1);

        for (int i = 0; i <= frequency; ++i) {
            glm::vec3 a = interpolate(v2, v0, i, frequency);
            glm::vec3 b = interpolate(v2, v1, i, frequency);

            for (int j = 0; j <= i; ++j) {
                glm::vec3 p = interpolate(a, b, j, i == 0 ? 1 : i);
                positions.push_back(p);
                grid[i].push_back((int)positions.size() - 1);
            }
        }

        // Create small triangles from the grid of points
        for (int i = 0; i < frequency; ++i) {
            for (int j = 0; j <= i; ++j) {
                int a = grid[i][j];
                int b = grid[i + 1][j];
                int c = grid[i + 1][j + 1];
                tris.push_back({a, b, c});

                if (j > 0) {
                    int d = grid[i][j - 1];
                    tris.push_back({a, d, b});
                }
            }
        }
    }

    // Deduplicate vertices
    std::vector<glm::vec3> uniqueVerts;
    std::vector<int> remap(positions.size(), -1);
    float eps = 1e-5f;
    for (size_t i = 0; i < positions.size(); ++i) {
        const glm::vec3& p = positions[i];
        bool found = false;
        for (size_t j = 0; j < uniqueVerts.size(); ++j) {
            if (glm::length(uniqueVerts[j] - p) < eps) {
                remap[i] = (int)j;
                found = true;
                break;
            }
        }
        if (!found) {
            remap[i] = (int)uniqueVerts.size();
            uniqueVerts.push_back(p);
        }
    }
    for (auto& tri : tris) {
        tri.x = remap[tri.x];
        tri.y = remap[tri.y];
        tri.z = remap[tri.z];
    }

    // Build vertex list
    mesh.vertices.reserve(uniqueVerts.size());
    for (const auto& p : uniqueVerts) {
        Vertex v;
        v.position = glm::normalize(p);
        v.normal = glm::normalize(p);
        v.texCoord = glm::vec2(0.0f);
        mesh.vertices.push_back(v);
    }

    mesh.triangles = tris;
    mesh.material.kd = glm::vec3(0.8f, 0.6f, 0.3f);
    return mesh;
}

int main() {
    Application app;
    app.update();

    return 0;
}
