#include "Battlecruiser.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <random>
#include <iostream>
#include <glm/gtx/quaternion.hpp>

Battlecruiser::Battlecruiser(Window& window): window(window) {
    const std::vector<Mesh> meshes = loadMesh(RESOURCE_ROOT "resources/BattleCruiser.obj");
    
    for (const auto& mesh : meshes)
    {
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triangles.size() * sizeof(glm::uvec3), mesh.triangles.data(), GL_STATIC_DRAW);

        // Vertex attributes
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2); //tex Coordinates
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

        glBindVertexArray(0);

        m.indexCount = mesh.triangles.size() * 3;
        m.materialName = mesh.material.name;

        std::cout << "Mesh detected: " << m.materialName << std::endl;
        std::cout << "Mesh vertices: " << mesh.vertices.size()
            << " triangles: " << mesh.triangles.size() << std::endl;

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
                        "shaders/battlecruiser/shader_vert.glsl")
            .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
                        "shaders/battlecruiser/glass_shader_frag.glsl")
            .build();
}

void Battlecruiser::draw(const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& lightPos,
    const glm::vec3& cameraPos,
    unsigned int cubemapTexture)
{
    // --- Setup once per frame ---
    LightParticle thruster = {
        glm::vec3(0.0f, 4.0f, -50.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(1.0f, 0.5f, 0.1f),
        glm::radians(80.0f),
        45.0f,
        4.5f
    };

    // Disable face culling to render inside the windows
    glDisable(GL_CULL_FACE); 

    // --- Pass 1: opaque meshes ---
    mainShader.bind();

    glUniformMatrix4fv(mainShader.getUniformLocation("model"), 1, GL_FALSE,
                       glm::value_ptr(getModelMatrix()));
    glUniformMatrix4fv(mainShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(mainShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(mainShader.getUniformLocation("lightPos"), 1, glm::value_ptr(lightPos));

    glUniform3fv(mainShader.getUniformLocation("thrusterLightPos"), 1, glm::value_ptr(thruster.pos));
    glUniform3fv(mainShader.getUniformLocation("thrusterLightDir"), 1, glm::value_ptr(thruster.dir));
    glUniform3fv(mainShader.getUniformLocation("thrusterLightColor"), 1, glm::value_ptr(thruster.color));
    glUniform1f(mainShader.getUniformLocation("thrusterThresholdLight"), thruster.thresholdLight);
    glUniform1f(mainShader.getUniformLocation("thrusterLightIntensity"), thruster.intensity);
    glUniform1f(mainShader.getUniformLocation("thrusterLightAngle"), thruster.angle);

    // Draw all opaque meshes that use the main shader
    for (const auto& m : meshGLs) {
        if (m.materialName == "Steel_-_Satin") {
            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
        }
    }

    // --- Pass 2: reflective meshes ---
    // Skip depth testing to avoid artifacts inside the windows
    glDepthMask(GL_FALSE);

    reflectiveShader.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glUniform1i(reflectiveShader.getUniformLocation("environmentMap"), 0);
    glUniform3fv(reflectiveShader.getUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPos));

    glUniformMatrix4fv(reflectiveShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(getModelMatrix()));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (const auto& m : meshGLs) {
        if (m.materialName == "Window-Cabin-Material") {
            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
        }
    }
}

void Battlecruiser::updateVelocityPosition(float deltaTime) {
    static float currentBankAngle = 0.0f;

    float initialSpeed = glm::length(velocity);

    const float bankSensitivity = glm::mix(0.4f, 1.0f, initialSpeed / 3);
    const float maxBankAngle = glm::mix(glm::radians(5.0f), glm::radians(40.0f), initialSpeed / 3);
    float sensitivityRotationX = glm::mix(0.2f, 1.8f, initialSpeed / 3);
    float sensitivityRotationY = glm::mix(0.2f, 0.8f, initialSpeed / 3);

    glm::vec3 directionVector = getDirectionVector();

    if (window.isKeyPressed(GLFW_KEY_LEFT)) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), deltaTime * sensitivityRotationX, glm::vec3(0.0f, 1.0f, 0.0f))) * directionVector;
    }
    if (window.isKeyPressed(GLFW_KEY_RIGHT)) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), -deltaTime * sensitivityRotationX, glm::vec3(0.0f, 1.0f, 0.0f))) * directionVector;
    }

    float maxPitch = glm::radians(80.0f);
    float pitchAngle = glm::asin(glm::dot(directionVector, getUpVector()));
    glm::vec3 right = glm::normalize(glm::cross(directionVector, getUpVector()));

    if (window.isKeyPressed(GLFW_KEY_UP) && pitchAngle < maxPitch) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), +deltaTime * sensitivityRotationY, right)) * directionVector;
    }
    if (window.isKeyPressed(GLFW_KEY_DOWN) && pitchAngle > -maxPitch) {
        directionVector = glm::mat3(glm::rotate(glm::mat4(1.0f), -deltaTime * sensitivityRotationY, right)) * directionVector;
    }

    if (window.isKeyPressed(GLFW_KEY_KP_ADD)) {
        initialSpeed += 0.1f;
    }
    if (window.isKeyPressed(GLFW_KEY_KP_SUBTRACT)) {
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

glm::vec3 Battlecruiser::getDirectionVector() {
    return glm::normalize(velocity);
}

glm::vec3 Battlecruiser::getUpVector() {
    return glm::normalize(upVector);
}


Battlecruiser::~Battlecruiser() {
    for (const MeshGL& m : meshGLs)
    {
        glDeleteBuffers(1, &m.vbo);
        glDeleteBuffers(1, &m.ibo);
        glDeleteVertexArrays(1, &m.vao);
    }
}