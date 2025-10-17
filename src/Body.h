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
        if(parent == nullptr) {
            position = glm::vec3(0.0f); // no orbiting if no parent
            return; // no orbiting if no parent
        }
        float focalDistance = glm::sqrt(largeRadius * largeRadius - smallRadius * smallRadius);
        glm::vec3 center = parent->getPosition() + focalDistance * orbit_direction;
        orbitAngle += (2.0f * glm::pi<float>() / orbitPeriod) * deltaTime; // radians per second
        if(orbitAngle > 2.0f * glm::pi<float>()) {
            orbitAngle -= 2.0f * glm::pi<float>();
        }
        glm::vec3 majorAxis = glm::normalize(orbit_direction);
        glm::vec3 minorAxis = glm::normalize(glm::cross(orbitNormal, majorAxis));
        position = center + largeRadius * cos(orbitAngle) * majorAxis + smallRadius * sin(orbitAngle) * minorAxis;

    }

    glm::mat4 get_model_matrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        return modelMatrix;
    }

    void imGuiControl() {
        // ImGui::DragFloat3("Planet Position", glm::value_ptr(position), 0.1f);
        ImGui::SliderFloat("Planet Radius", &radius, 0.1f, 10.0f, "%.2f");
        ImGui::SliderFloat("Test", &test, 0.0f, 100.00f, "%.2f");
        ImGui::Separator();
        ImGui::Text("Orbit Controls");
        ImGui::SliderFloat3("Orbit direction",
                            glm::value_ptr(param_orbit_direction), -1.0f, 1.0f,
                            "%.2f");
        ImGui::SliderFloat("Small Radius", &param_small_radius, 0.0f, 100.0f,
                           "%.2f");
        ImGui::SliderFloat("Large Radius", &param_large_radius, 0.0f, 100.0f,
                           "%.2f");
        ImGui::SliderFloat3("Orbit Normal", glm::value_ptr(param_orbit_normal),
                            -1.0f, 1.0f, "%.3f");
        ImGui::SliderFloat("Orbit Period (s)", &param_orbit_period, 0.001f,
                           10.0f, "%.2f");
        if(ImGui::Button("Apply Orbit Changes")){
            set_orbit(param_orbit_direction, param_small_radius,
                      param_large_radius, param_orbit_normal,
                      param_orbit_period, parent);
        }
    }

    void draw(const Shader& drawingShader) const {
        glUniform1f(drawingShader.getUniformLocation("radius"), radius);
        glUniform1f(drawingShader.getUniformLocation("test"), test);

        icosahedronMesh.drawPatches(drawingShader);
    }

    void set_orbit(const glm::vec3& direction, float small_r, float large_r, const glm::vec3& normal, float period, Body* parent_body) {
        orbit_direction = glm::normalize(direction - glm::dot(direction, orbitNormal) * orbitNormal);
        smallRadius = small_r;
        largeRadius = large_r;
        orbitNormal = glm::normalize(normal);
        orbitPeriod = period;
        parent = parent_body;
        // orbitAngle = 0.0f;            
        param_orbit_direction = orbit_direction;
        param_small_radius = smallRadius;
        param_large_radius = largeRadius;
        param_orbit_normal = orbitNormal;
        param_orbit_period = orbitPeriod;
    }

   private:
    GPUMesh& icosahedronMesh;
    glm::vec3 position;
    float radius;
    float test = 0.0f;

    // Orbit parameters
    Body* parent = nullptr; // the body this one orbits around
    glm::vec3 orbit_direction; // direction of orbit (not necessarily normalized)
    float smallRadius; // small elliptical orbit radius
    float largeRadius; // large elliptical orbit radius
    glm::vec3 orbitNormal; // normal vector of the orbital plane
    // major axis is aligned with orbit_direction ( while perpendicular to orbitNormal,
    // so minor axis is perpendicular to orbit_direction in the orbital plane)
    float orbitPeriod; // in seconds
    float orbitAngle{0.0f}; // current angle in the orbit, in radians (0 means towards R)

    glm::vec3 param_orbit_direction {1.0f, 0.0f, 0.0f};
    float param_small_radius = 1.0f;
    float param_large_radius = 1.0f;
    glm::vec3 param_orbit_normal {0.0f, 1.0f, 0.0f};
    float param_orbit_period = 1.0f;
};