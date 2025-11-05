#include "Battlecruiser.h"
#include "BezierPath.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <random>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <imgui/imgui.h>
#include <stb/stb_image.h>

void Battlecruiser::loadTexture(const char *filename, GLuint &texture) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (!data)
        std::cerr << "Failed to load texture: " << filename << std::endl;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
}


Battlecruiser::Battlecruiser(Window &window, Config &config): window(window), config(config) {
    const std::vector<Mesh> meshes = loadMesh(RESOURCE_ROOT "resources/BattleCruiser/Untitled.obj");

    for (const auto &mesh: meshes) {
        MeshGL m;

        // Create VAO
        glGenVertexArrays(1, &m.vao);
        glBindVertexArray(m.vao);

        // Create and upload vertex buffer
        glGenBuffers(1, &m.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

        // Create and upload index buffer
        glGenBuffers(1, &m.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triangles.size() * sizeof(glm::uvec3), mesh.triangles.data(),
                     GL_STATIC_DRAW);

        // Vertex attributes
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));

        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));

        glEnableVertexAttribArray(2); //tex Coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, texCoord));

        glEnableVertexAttribArray(3); //tangent for TBN
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tangent));

        m.indexCount = mesh.triangles.size() * 3;
        m.materialName = mesh.material.name;

        if (m.materialName == "Panel-0") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-0/spaceship-panels1-albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-0/spaceship-panels1-normal-ogl.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-0/spaceship-panels1-metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-0/spaceship-panels1-roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-1") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-1/TCom_Scifi_Panel_4K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-1/TCom_Scifi_Panel_4K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-1/TCom_Scifi_Panel_4K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-1/TCom_Scifi_Panel_4K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-2") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-2/TCom_SciFiPanels_02_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-2/TCom_SciFiPanels_02_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-2/TCom_SciFiPanels_02_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-2/TCom_SciFiPanels_02_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-3") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-3/TCom_SciFiPanels06_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-3/TCom_SciFiPanels06_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-3/TCom_SciFiPanels06_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-3/TCom_SciFiPanels06_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-4") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-4/TCom_Scifi_Panel8_New_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-4/TCom_Scifi_Panel8_New_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-4/TCom_Scifi_Panel8_New_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-4/TCom_Scifi_Panel8_New_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-5") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-5/TCom_ScratchedSteel2_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-5/TCom_ScratchedSteel2_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-5/TCom_ScratchedSteel2_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-5/TCom_ScratchedSteel2_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-6") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-6/TCom_SciFiPanels07_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-6/TCom_SciFiPanels07_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-6/TCom_SciFiPanels07_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-6/TCom_SciFiPanels07_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-7") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-7/TCom_SciFiPanels09_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-7/TCom_SciFiPanels09_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-7/TCom_SciFiPanels09_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-7/TCom_SciFiPanels09_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-8") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-8/TCom_SciFiPanels03_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-8/TCom_SciFiPanels03_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-8/TCom_SciFiPanels03_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-8/TCom_SciFiPanels03_1K_roughness.png", m.roughnessMap);
        }
        if (m.materialName == "Panel-9") {
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-9/TCom_SciFiPanels_01_1K_albedo.png", m.baseMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-9/TCom_SciFiPanels_01_1K_normal.png", m.normalMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-9/TCom_SciFiPanels_01_1K_metallic.png", m.metallicMap);
            loadTexture(RESOURCE_ROOT "resources/BattleCruiser/panel-9/TCom_SciFiPanels_01_1K_roughness.png", m.roughnessMap);
        }

        glBindVertexArray(0);

        meshGLs.push_back(m);
    }

    mainShader =
            ShaderBuilder()
            .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/shader_vert.glsl")
            .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/shader_frag.glsl")
            .build();
    reflectiveShader =
            ShaderBuilder()
            .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/glass_shader_vert.glsl")
            .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
                      "shaders/battlecruiser/glass_shader_frag.glsl")
            .build();

    thrusterShader =
        ShaderBuilder()
            .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/battlecruiser/shader_vert.glsl")
            .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/battlecruiser/thruster_frag.glsl")
            .build();
}

void Battlecruiser::draw(const glm::mat4 &view,
                         const glm::mat4 &projection,
                         const glm::vec3 &lightPos,
                         const glm::vec3 &cameraPos,
                         unsigned int cubemapTexture) {

    // TODO Need to add light correction
    glm::vec3 lightColor = glm::vec3(4.0f);

    // Disable face culling to render inside the windows
    glDisable(GL_CULL_FACE);

    // --- Pass 1: opaque meshes ---
    mainShader.bind();

    glUniformMatrix4fv(mainShader.getUniformLocation("model"), 1, GL_FALSE,
                       glm::value_ptr(getModelMatrix()));
    glUniformMatrix4fv(mainShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mainShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(mainShader.getUniformLocation("lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(mainShader.getUniformLocation("lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(mainShader.getUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPos));

    // Draw all opaque meshes that use the main shader
    for (const auto &m: meshGLs) {
        if (m.materialName != "Panel-10") {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m.baseMap);
            glUniform1i(mainShader.getUniformLocation("textureBase"), 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m.normalMap);
            glUniform1i(mainShader.getUniformLocation("textureNormal"), 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m.metallicMap);
            glUniform1i(mainShader.getUniformLocation("textureMetallic"), 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, m.roughnessMap);
            glUniform1i(mainShader.getUniformLocation("textureRoughness"), 3);

            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    // --- Pass 2: reflective meshes ---
    reflectiveShader.bind();
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("model"), 1, GL_FALSE,
                       glm::value_ptr(getModelMatrix()));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(reflectiveShader.getUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPos));

    for (const auto &m: meshGLs) {
        if (m.materialName == "Panel-10") {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubemapTexture);
            glUniform1i(mainShader.getUniformLocation("environmentMap"), 0);

            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
        }
    }

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    // --- Pass 3: Thruster Light ---
    static std::default_random_engine rng(std::random_device{}());
    static std::uniform_real_distribution<float> flickerDist(-0.01f, 0.01f);

    std::uniform_int_distribution<int> distR(233.0f, 255.f);
    std::uniform_int_distribution<int> distG(165.0f, 255.f);

    glm::vec3 lightThrusterPos = glm::vec3(-0.02f, -0.147f, -2.2f);
    glm::vec3 lightThrusterColor = glm::vec3(static_cast<unsigned char>(distR(rng)), static_cast<unsigned char>(distG(rng)), 0.0f);
    glm::vec3 thrusterDir = getDirectionVector();

    float randomOffset = flickerDist(rng);
    float flickeringRadius = thrusterRadius + randomOffset;

    thrusterShader.bind();
    glUniformMatrix4fv(thrusterShader.getUniformLocation("model"), 1, GL_FALSE,
                       glm::value_ptr(getModelMatrix()));
    glUniformMatrix4fv(thrusterShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(thrusterShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(thrusterShader.getUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPos));

    glUniform3fv(thrusterShader.getUniformLocation("thrusterLightPos"), 1, glm::value_ptr(lightThrusterPos));
    glUniform3fv(thrusterShader.getUniformLocation("thrusterLightColor"), 1, glm::value_ptr(lightThrusterColor));
    glUniform3fv(thrusterShader.getUniformLocation("thrusterDir"), 1, glm::value_ptr(thrusterDir));

    glUniform1f(thrusterShader.getUniformLocation("radius"), radius);
    glUniform1i(thrusterShader.getUniformLocation("nrLights"), nrLights);
    glUniform1f(thrusterShader.getUniformLocation("thrusterRadius"), flickeringRadius);
    glUniform1f(thrusterShader.getUniformLocation("thrusterIntensity"), thrusterIntensity);


    for (const auto &m: meshGLs) {
        glBindVertexArray(m.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
    }
}

void Battlecruiser::drawBezierPath(const glm::mat4 &view,
              const glm::mat4 &projection) {
    if (isFollowingPath) {
        glm::vec2 viewport = window.getFrameBufferSize();

        bezierPath.draw(view, projection, modelMatrix, viewport);
    }
}

void Battlecruiser::updatePosition(float deltaTime) {
    if (isFollowingPath) {
        updateVelocityPositionPathMovement(deltaTime);
    } else {
        updateVelocityPositionFreeMovement(deltaTime);
    }
}

void Battlecruiser::updateVelocityPositionPathMovement(float deltaTime) {
    bezierPath.updateVelocityPositionPathMovement(deltaTime);

    position = bezierPath.getCurrentPosition();
    velocity = bezierPath.getCurrentVelocity();
}


void Battlecruiser::updateVelocityPositionFreeMovement(float deltaTime) {
    static float currentBankAngle = 0.0f;

    float initialSpeed = glm::length(velocity);

    const float bankSensitivity = glm::mix(0.4f, 1.0f, initialSpeed / 3);
    const float maxBankAngle = glm::mix(glm::radians(5.0f), glm::radians(40.0f), initialSpeed / 3);
    float sensitivityRotationX = glm::mix(0.2f, 1.8f, initialSpeed / 3);
    float sensitivityRotationY = glm::mix(0.2f, 0.8f, initialSpeed / 3);

    glm::vec3 directionVector = getDirectionVector();

    if (window.isKeyPressed(GLFW_KEY_LEFT)) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), deltaTime * sensitivityRotationX,
                                                glm::vec3(0.0f, 1.0f, 0.0f))) * directionVector;
    }
    if (window.isKeyPressed(GLFW_KEY_RIGHT)) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), -deltaTime * sensitivityRotationX,
                                                glm::vec3(0.0f, 1.0f, 0.0f))) * directionVector;
    }

    float maxPitch = glm::radians(80.0f);
    float pitchAngle = glm::asin(glm::dot(directionVector, getUpVector()));
    glm::vec3 right = glm::normalize(glm::cross(directionVector, getUpVector()));

    if (window.isKeyPressed(GLFW_KEY_UP) && pitchAngle < maxPitch) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), +deltaTime * sensitivityRotationY, right)) *
                          directionVector;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN) && pitchAngle > -maxPitch) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), -deltaTime * sensitivityRotationY, right)) *
                          directionVector;
    }

    if (window.isKeyPressed(GLFW_KEY_PAGE_UP)) {
        initialSpeed += 0.1f;
    }
    if (window.isKeyPressed(GLFW_KEY_PAGE_DOWN)) {
        initialSpeed -= 0.1f;
    }

    if (initialSpeed > 3.0f) {
        initialSpeed = 3.0f;
    }
    if (initialSpeed < 0.2f) {
        initialSpeed = 0.2f;
    }
    directionVector = glm::normalize(directionVector);
    velocity = directionVector * initialSpeed;
    position += velocity * deltaTime;


    float targetBankAngle = 0.0f;
    if (window.isKeyPressed(GLFW_KEY_RIGHT))
        targetBankAngle = maxBankAngle;
    else if (window.isKeyPressed(GLFW_KEY_LEFT))
        targetBankAngle = -maxBankAngle;

    currentBankAngle = glm::mix(currentBankAngle, targetBankAngle, deltaTime * bankSensitivity);

    glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), currentBankAngle, directionVector);
    glm::vec3 bankedUp = glm::normalize(glm::vec3(rollMatrix * glm::vec4(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f)));

    upVector = bankedUp;
}


std::vector<glm::vec3> Battlecruiser::getRelativePositionThrusters() {
    return relativePositionThrusters;
}

glm::mat4 Battlecruiser::getModelMatrix() {
    glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);

    glm::vec3 dirV = getDirectionVector();
    glm::vec3 upV = getUpVector();
    if (glm::abs(glm::dot(dirV, upV)) > 0.999f) {
        upV = glm::normalize(glm::abs(dirV.x) < 0.9f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0));
    }

    glm::vec3 right = glm::normalize(cross(upV, dirV));
    glm::vec3 upOrt = glm::cross(dirV, right);
    glm::vec3 fwd = dirV;

    glm::mat4 rotationMat(1.0f);
    rotationMat[0] = glm::vec4(right, 0.0f);
    rotationMat[1] = glm::vec4(upOrt, 0.0f);
    rotationMat[2] = glm::vec4(fwd, 0.0f);

    return translationMat * rotationMat * modelMatrix;
}

float Battlecruiser::getSpeed() const {
    return glm::length(velocity);
}


glm::vec3 Battlecruiser::getDirectionVector() {
    return glm::normalize(velocity);
}

glm::vec3 Battlecruiser::getUpVector() {
    return glm::normalize(upVector);
}

void Battlecruiser::imGuiControl() {
    ImGui::Separator();
    ImGui::Text("Battlecruiser Movement");
    if (ImGui::Checkbox("Follow Bezier Path", &isFollowingPath)) {
        bezierPath.initializeBezierPathMovement(config.battlecruiser_path_info, position, velocity);
    }

    ImGui::Separator();
    ImGui::Text("Battlecruiser Light Panel");
    ImGui::DragFloat("Radius", &radius, 0.01f);
    ImGui::DragInt("Nr Lights", &nrLights, 1);
    ImGui::DragFloat("Thruster radius", &thrusterRadius, 0.01f);
    ImGui::DragFloat("Thruster intensity", &thrusterIntensity, 0.01f);
}


Battlecruiser::~Battlecruiser() {
    for (const MeshGL &m: meshGLs) {
        glDeleteBuffers(1, &m.vbo);
        glDeleteBuffers(1, &m.ibo);
        glDeleteVertexArrays(1, &m.vao);
    }
}

