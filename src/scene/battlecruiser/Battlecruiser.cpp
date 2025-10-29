#include "Battlecruiser.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <random>
#include <iostream>

Battlecruiser::Battlecruiser() {
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
    thruster = {
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
                       glm::value_ptr(modelMatrix));
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

    glm::mat4 mvp = projection * view * modelMatrix;
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(reflectiveShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (const auto& m : meshGLs) {
        if (m.materialName == "Window-Cabin-Material") {
            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m.indexCount), GL_UNSIGNED_INT, nullptr);
        }
    }
}


std::vector<glm::vec3> Battlecruiser::getRelativePositionThrusters() {
    return relativePositionThrusters;
}

glm::mat4 Battlecruiser::getModelMatrix() {
    return modelMatrix;
}

Battlecruiser::~Battlecruiser() {
    for (const MeshGL& m : meshGLs)
    {
        glDeleteBuffers(1, &m.vbo);
        glDeleteBuffers(1, &m.ibo);
        glDeleteVertexArrays(1, &m.vao);
    }
}