// #include "Image.h"
#include "Body.h"
#include "mesh.h"
#include "texture.h"
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

// Forward declaration for the helper defined below
static Mesh makeGeodesicIcosahedronMesh(int frequency);

class Application {
   public:
    Application()
        : m_window("Final Project", glm::ivec2(1024, 1024),
                   OpenGLVersion::GL41),
          m_texture(RESOURCE_ROOT "resources/checkerboard.png") {
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

        // Load model meshes (from file) and also create a hardcoded icosahedron
        // mesh
        m_modelMeshes =
            GPUMesh::loadMeshGPU(RESOURCE_ROOT "resources/dragon.obj");

        // Create a hardcoded icosahedron CPU mesh and upload to GPU
        Mesh ico = makeGeodesicIcosahedronMesh(10);
        try {
            m_icosaMesh = GPUMesh(ico);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create icosahedron GPUMesh: " << e.what()
                      << std::endl;
        }

        m_bodies.emplace_back(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, m_icosaMesh);

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
        static double lastTime = glfwGetTime();
        while (!m_window.shouldClose()) {
            // This is your game loop
            // Put your real-time logic and rendering in here
            m_window.updateInput();


            // Update camera position 
            double deltaTime = glfwGetTime() - lastTime;
            lastTime = glfwGetTime();
            update_camera(deltaTime);
                
            // Use ImGui for easy input/output of ints, floats, strings, etc...
            ImGui::Begin("Window");
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

            // Render mode combo: 0 = loaded model (default), 1 = icosahedron
            static const char* meshItems[] = {"Loaded model", "Icosahedron"};
            ImGui::Combo("Render Mode", &m_renderMode, meshItems,
                         IM_ARRAYSIZE(meshItems));

            m_bodies[0].imGuiControl();

            ImGui::End();

            renderFrame();

            // Processes input and swaps the window buffer
            m_window.swapBuffers();
        }
    }

    void update_camera(double delta_time){
        // Recompute view matrix to always look at origin (0,0,0)
        const float yawRad = glm::radians(m_cameraYaw);
        const float pitchRad = glm::radians(m_cameraPitch);

        m_cameraFront = glm::vec3(
            cosf(pitchRad) * cosf(yawRad),
            sinf(pitchRad),
            cosf(pitchRad) * sinf(yawRad)
        );

        glm::vec3 right = glm::normalize(glm::cross(m_cameraFront, m_cameraUp));

        float camera_speed = 0.5f;

        if(m_isMovingLeft){
            m_cameraPosition += right * -camera_speed * (float) delta_time;
        }
        if(m_isMovingRight){
            m_cameraPosition += right * camera_speed * (float) delta_time;
        }

        m_cameraPosition += glm::normalize(m_cameraFront) * m_scrollOffset;
        m_scrollOffset = 0.0f;

        m_viewMatrix = glm::lookAt(m_cameraPosition, m_cameraPosition + m_cameraFront, m_cameraUp);
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__keys.html mods - Any modifier
    // keys pressed, like shift or control
    void onKeyPressed(int key, int mods) {
        if (key == GLFW_KEY_A)
            m_isMovingLeft = true;
        else if (key == GLFW_KEY_D)
            m_isMovingRight = true;

        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__keys.html mods - Any modifier
    // keys pressed, like shift or control
    void onKeyReleased(int key, int mods) {
        if(key == GLFW_KEY_A)
            m_isMovingLeft = false;
        else if(key == GLFW_KEY_D)
            m_isMovingRight = false;
        
        std::cout << "Key released: " << key << std::endl;
    }

    // If the mouse is moved this function will be called with the x, y
    // screen-coordinates of the mouse
    void onMouseMove(const glm::dvec2& cursorPos) {
        // If dragging, orbit the camera around the origin.
        if (m_isDragging) {
            // Initialize previous cursor if invalid
            if (m_prevCursor.x < 0.0 && m_prevCursor.y < 0.0) {
                m_prevCursor = cursorPos;
                return;
            }

            const double dx = cursorPos.x - m_prevCursor.x;
            const double dy = cursorPos.y - m_prevCursor.y;
            // Sensitivity: degrees per pixel
            const double yawSpeed = 0.1;
            const double pitchSpeed = 0.1;

            m_cameraYaw += static_cast<float>(dx * yawSpeed);
            m_cameraPitch += static_cast<float>(dy * pitchSpeed);

            // Clamp pitch to avoid flipping (e.g., -89..89 degrees)
            if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
            if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

            m_prevCursor = cursorPos;
        }
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__buttons.html mods - Any modifier
    // buttons pressed
    void onMouseClicked(int button, int mods) {
        // Start dragging when left mouse button is pressed
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            m_isDragging = true;
            // Invalidate previous cursor so the first movement doesn't jump
            m_prevCursor = glm::dvec2(-1.0, -1.0);
        }
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in
    // https://www.glfw.org/docs/latest/group__buttons.html mods - Any modifier
    // buttons pressed
    void onMouseReleased(int button, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            m_isDragging = false;
            m_prevCursor = glm::dvec2(-1.0, -1.0);
        }
    }

    // Handle mouse scroll (e.g., for zooming)
    // offset.x - horizontal scroll, offset.y - vertical scroll (typically used
    // for zoom)
    void onMouseScroll(const glm::vec2& offset) {
        std::cout << "Mouse scroll: " << offset.x << " " << offset.y
                  << std::endl;
        // Zoom sensitivity: units per scroll tick
        const float zoomSpeed = 0.2f;

        m_scrollOffset += offset.y * zoomSpeed;
    }

    // Render the current frame (clears, sets states, and draws selected meshes)
    void renderFrame() {
        // Simple dispatcher: call the appropriate full-mode renderer
        if (m_renderMode == 0) {
            renderDefault();
        } else {
            renderIco();
        }
    }

    // Full default renderer: clears and renders all loaded model meshes
    // (matches original behavior)
    void renderDefault() {
        // Clear the screen
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        const glm::mat4 mvpMatrix =
            m_projectionMatrix * m_viewMatrix * m_modelMatrix;
        // Normals should be transformed differently than positions (ignoring
        // translations + dealing with scaling):
        // https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
        const glm::mat3 normalModelMatrix =
            glm::inverseTranspose(glm::mat3(m_modelMatrix));

        for (GPUMesh& mesh : m_modelMeshes) {
            // Bind shader and upload matrices
            m_defaultShader.bind();
            glUniformMatrix4fv(m_defaultShader.getUniformLocation("mvpMatrix"),
                               1, GL_FALSE, glm::value_ptr(mvpMatrix));
            // Uncomment this line when you use the modelMatrix (or
            // fragmentPosition)
            // glUniformMatrix4fv(m_defaultShader.getUniformLocation("modelMatrix"),
            // 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
            glUniformMatrix3fv(
                m_defaultShader.getUniformLocation("normalModelMatrix"), 1,
                GL_FALSE, glm::value_ptr(normalModelMatrix));

            // Material / texture handling and draw (inlined)
            if (mesh.hasTextureCoords()) {
                m_texture.bind(GL_TEXTURE0);
                glUniform1i(m_defaultShader.getUniformLocation("colorMap"), 0);
                glUniform1i(m_defaultShader.getUniformLocation("hasTexCoords"),
                            GL_TRUE);
                glUniform1i(m_defaultShader.getUniformLocation("useMaterial"),
                            GL_FALSE);
            } else {
                glUniform1i(m_defaultShader.getUniformLocation("hasTexCoords"),
                            GL_FALSE);
                glUniform1i(m_defaultShader.getUniformLocation("useMaterial"),
                            m_useMaterial);
            }
            mesh.draw(m_defaultShader);
        }
    }

    // Full icosahedron renderer: clears and renders the procedural icosahedron
    // meshes (wireframe respected)
    void renderIco() {
        // Clear the screen (keep same background as default)
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDisable(GL_CULL_FACE);

        // Respect wireframe toggle
        if (m_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        for (Body& body : m_bodies) {
            glm::mat4 modelMatrix = body.get_model_matrix();

            const glm::mat4 mvpMatrix =
                m_projectionMatrix * m_viewMatrix * modelMatrix;

            m_icoShader.bind();
            glUniformMatrix4fv(m_icoShader.getUniformLocation("mvpMatrix"), 1,
                GL_FALSE, glm::value_ptr(mvpMatrix));
            glUniform3fv(m_icoShader.getUniformLocation("cameraWorldPos"), 1,
                glm::value_ptr(m_cameraPosition));
            glUniform1i(m_icoShader.getUniformLocation("tessellate"),
                        m_bodyTessellation ? 1 : 0);

            body.draw(m_icoShader);
        }
    }

   private:
    Window m_window;

    // Shader for default rendering and for depth rendering
    Shader m_defaultShader;
    Shader m_shadowShader;
    // Shader with tessellation for rendering the bodies
    Shader m_icoShader;

    // Keep separate collections for model meshes and the procedurally created
    // icosahedron
    std::vector<GPUMesh> m_modelMeshes;
    GPUMesh m_icosaMesh{Mesh{}};
    int m_renderMode{0};
    Texture m_texture;
    bool m_useMaterial{true};
    bool m_wireframe{false};
    bool m_bodyTessellation{true};

    std::vector<Body> m_bodies;

    // Camera parameters
    float m_cameraYaw{-45.0f};   // degrees
    float m_cameraPitch{30.0f};  // degrees
    glm::vec3 m_cameraPosition{-1, 1, -1};
    glm::vec3 m_cameraUp{0,1,0};
    glm::vec3 m_cameraFront{0,0,0};
    // Projection and view matrices for you to fill in and use
    glm::mat4 m_projectionMatrix =
        glm::perspective(glm::radians(80.0f), 1.0f, 0.1f, 30.0f);
    glm::mat4 m_viewMatrix =
        glm::lookAt(m_cameraPosition, glm::vec3(0), m_cameraUp);
    glm::mat4 m_modelMatrix{1.0f};
    // Input state
    glm::dvec2 m_prevCursor{-1.0, -1.0};
    float m_scrollOffset{0.0f};
    bool m_isDragging{false};
    bool m_isMovingLeft{false};
    bool m_isMovingRight{false};
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
