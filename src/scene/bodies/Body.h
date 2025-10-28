#pragma once
#include <framework/disable_all_warnings.h>
#include <framework/shader.h>

DISABLE_WARNINGS_PUSH()
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <iostream>

#include "core/ShadowMap.h"
#include "core/config.h"
#include "core/mesh.h"

// This is a planet / star / space body base class with primitive default
// behavior.
class Body {
   private:
    Config& config;
    ShadowMap shadow_map;

   public:
    Body(Config& config, const glm::vec3& pos, float r,
         GPUMesh& icosahedron_mesh)
        : config(config),
          position(pos),
          radius(r),
          icosahedronMesh(icosahedron_mesh),
          shadow_map(config) {}

    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }

    virtual void setup() {
        try {
            ShaderBuilder icoBuilder;
            icoBuilder.addStage(GL_VERTEX_SHADER,
                                RESOURCE_ROOT "shaders/bodies/ico_vert.glsl");
            icoBuilder.addStage(GL_TESS_CONTROL_SHADER,
                                RESOURCE_ROOT "shaders/bodies/ico_tesc.glsl");
            icoBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                                RESOURCE_ROOT "shaders/bodies/ico_tese.glsl");
            icoBuilder.addStage(GL_FRAGMENT_SHADER,
                                RESOURCE_ROOT "shaders/bodies/ico_frag.glsl");
            shader = icoBuilder.build();
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }
    }

    virtual void update(float deltaTime,
                        glm::vec3 p_light_position = glm::vec3(0.0f)) {
        // Update light matrices for shadow mapping
        light_position = p_light_position;
        light_view_matrix =
            glm::lookAt(light_position, position, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 centerLS =
            glm::vec3(light_view_matrix * glm::vec4(position, 1.0f));
        float near_plane = std::max(0.1f, -centerLS.z - radius * 2.0f);
        // near_plane = 0.1f;
        float far_plane = -centerLS.z + radius * 2.0f;
        // far_plane = 100.0f;

        light_projection_matrix =
            glm::ortho(-radius * 2.0f, radius * 2.0f, -radius * 2.0f,
                       radius * 2.0f, near_plane, far_plane);

        // Update body state based on deltaTime
        if (parent == nullptr) {
            position = glm::vec3(0.0f);  // no orbiting if no parent
            return;                      // no orbiting if no parent
        }
        // Focal distance: distance from ellipse center to focus (parent)
        float focalDistance =
            glm::sqrt(largeRadius * largeRadius - smallRadius * smallRadius);

        // Unit axes in the orbital plane
        glm::vec3 majorAxis = glm::normalize(orbit_direction);
        glm::vec3 minorAxis =
            glm::normalize(glm::cross(orbitNormal, majorAxis));

        // The ellipse center is offset from the parent along orbit_direction
        glm::vec3 center = parent->getPosition() + focalDistance * majorAxis;

        // Advance orbit angle at constant angular speed
        float angularSpeed =
            (2.0f * glm::pi<float>()) / orbitPeriod;  // radians/sec
        orbitAngle += angularSpeed * deltaTime;
        if (orbitAngle > 2.0f * glm::pi<float>())
            orbitAngle -= 2.0f * glm::pi<float>();

        // Compute position on the ellipse
        position = center + largeRadius * glm::cos(orbitAngle) * majorAxis +
                   smallRadius * glm::sin(orbitAngle) * minorAxis;
    }

    glm::mat4 get_model_matrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        return modelMatrix;
    }

    virtual void imGuiControl() {
        ImGui::DragFloat("Planet Radius", &radius, 0.01f, 0.1f, 10.0f, "%.2f");
        ImGui::DragFloat("Test", &test, 0.01f, 0.0f, 2.00f, "%.3f");
        ImGui::Separator();
        ImGui::Text("Orbit Controls");
        ImGui::DragFloat3("Orbit direction",
                          glm::value_ptr(param_orbit_direction), 0.01f, -1.0f,
                          1.0f, "%.2f");
        ImGui::DragFloat("Small Radius", &param_small_radius, 0.1f, 0.0f,
                         100.0f, "%.2f");
        ImGui::DragFloat("Large Radius", &param_large_radius, 0.1f, 0.0f,
                         100.0f, "%.2f");
        ImGui::DragFloat3("Orbit Normal", glm::value_ptr(param_orbit_normal),
                          0.01f, -1.0f, 1.0f, "%.3f");
        ImGui::DragFloat("Orbit Period (s)", &param_orbit_period, 0.01f, 0.001f,
                         100.0f, "%.2f");
        if (ImGui::Button("Apply Orbit Changes")) {
            set_orbit(param_orbit_direction, param_small_radius,
                      param_large_radius, param_orbit_normal,
                      param_orbit_period, parent);
        }
    }

    virtual void set_uniforms() {  // this assumes the shader is already bound
        glUniform1f(shader.getUniformLocation("radius"), radius);
        glUniform1f(shader.getUniformLocation("test"), test);
    }

    virtual bool needs_shadow_map() { return false; }

    void draw_depth(glm::vec3 light_position) {
        shader.bind();
        shadow_map.bind_for_writing();

        glm::mat4 modelMatrix = get_model_matrix();

        glUniformMatrix4fv(shader.getUniformLocation("model"), 1, GL_FALSE,
                           glm::value_ptr(modelMatrix));

        glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE,
                           glm::value_ptr(light_view_matrix));
        glUniformMatrix4fv(shader.getUniformLocation("projection"), 1, GL_FALSE,
                           glm::value_ptr(light_projection_matrix));
        glUniform1i(shader.getUniformLocation("only_depth"), 1);
        set_uniforms();

        icosahedronMesh.drawPatches(shader);
    }

    void draw(
        glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
        glm::vec3 cameraPos) {  // this assumes the shader is already bound
        shader.bind();
        glm::mat4 modelMatrix = get_model_matrix();
        glUniformMatrix4fv(shader.getUniformLocation("model"), 1, GL_FALSE,
                           glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE,
                           glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(shader.getUniformLocation("projection"), 1, GL_FALSE,
                           glm::value_ptr(projectionMatrix));
        glUniform3fv(shader.getUniformLocation("cameraWorldPos"), 1,
                     glm::value_ptr(cameraPos));
        glUniform3fv(shader.getUniformLocation("planet_center"), 1,
                     glm::value_ptr(position));
        glUniform1i(shader.getUniformLocation("only_depth"), 0);
        glUniformMatrix4fv(shader.getUniformLocation("lightViewMatrix"), 1,
                           GL_FALSE, glm::value_ptr(light_view_matrix));
        glUniformMatrix4fv(shader.getUniformLocation("lightProjectionMatrix"),
                           1, GL_FALSE,
                           glm::value_ptr(light_projection_matrix));
        glUniform3fv(shader.getUniformLocation("lightPosition"), 1,
                     glm::value_ptr(light_position));

        shadow_map.bind_for_reading(GL_TEXTURE5);
        glUniform1i(shader.getUniformLocation("shadowMap"), 5);

        set_uniforms();

        icosahedronMesh.drawPatches(shader);
    }

    void set_orbit(const glm::vec3& direction, float small_r, float large_r,
                   const glm::vec3& normal, float period, Body* parent_body) {
        orbitNormal = glm::normalize(normal);
        orbit_direction = glm::normalize(
            direction - glm::dot(direction, orbitNormal) * orbitNormal);
        smallRadius = small_r;
        largeRadius = large_r;
        orbitPeriod = period;
        parent = parent_body;
        param_orbit_direction = orbit_direction;
        param_small_radius = smallRadius;
        param_large_radius = largeRadius;
        param_orbit_normal = orbitNormal;
        param_orbit_period = orbitPeriod;
    }

    Shader shader;

   protected:
    GPUMesh& icosahedronMesh;
    glm::vec3 position;
    float radius;
    float test = 0.1f;

    // Orbit parameters
    Body* parent = nullptr;  // the body this one orbits around
    glm::vec3
        orbit_direction;    // direction of orbit (not necessarily normalized)
    float smallRadius;      // small elliptical orbit radius
    float largeRadius;      // large elliptical orbit radius
    glm::vec3 orbitNormal;  // normal vector of the orbital plane
    // major axis is aligned with orbit_direction ( while perpendicular to
    // orbitNormal, so minor axis is perpendicular to orbit_direction in the
    // orbital plane)
    float orbitPeriod;  // in seconds
    float orbitAngle{
        0.0f};  // current angle in the orbit, in radians (0 means towards R)

    glm::vec3 param_orbit_direction{1.0f, 0.0f, 0.0f};
    float param_small_radius = 1.0f;
    float param_large_radius = 1.0f;
    glm::vec3 param_orbit_normal{0.0f, 1.0f, 0.0f};
    float param_orbit_period = 1.0f;

    // light params
    glm::mat4 light_view_matrix;
    glm::vec3 light_position;
    glm::mat4 light_projection_matrix;
};