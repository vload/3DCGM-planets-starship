#pragma once
#include <framework/disable_all_warnings.h>
#include <framework/shader.h>

#include "mesh.h"
DISABLE_WARNINGS_PUSH()
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

// This is a planet
// TODO: create a Planet class that inherits from Body, and also Star maybe
// Asteroid / moon? IDK
class Body {
   public:
    Body(const glm::vec3& pos, float r, GPUMesh& icosahedron_mesh)
        : position(pos), radius(r), icosahedronMesh(icosahedron_mesh) {}

    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }

    void update(float deltaTime) {
        // Update body state based on deltaTime
        // nothing for now
    }

    glm::mat4 get_model_matrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        return modelMatrix;
    }

    void imGuiControl() {
        ImGui::DragFloat3("Planet Position", glm::value_ptr(position), 0.1f);
        ImGui::SliderFloat("Planet Radius", &radius, 0.1f, 100.0f, "%.2f");
        ImGui::SliderFloat("Test", &test, 0.0f, 100.00f, "%.2f");
    }

    void draw(const Shader& drawingShader) const {
        glUniform1f(drawingShader.getUniformLocation("radius"), radius);
        glUniform1f(drawingShader.getUniformLocation("test"), test);

        icosahedronMesh.drawPatches(drawingShader);
    }

   private:
    GPUMesh& icosahedronMesh;
    glm::vec3 position;
    float radius;
    float test = 0.0f;
};