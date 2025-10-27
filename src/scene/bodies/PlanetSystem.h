#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>
DISABLE_WARNINGS_POP()

#include "core/config.h"
#include "core/mesh.h"
#include "scene/bodies/Body.h"
#include "scene/bodies/Earth.h"
#include "scene/bodies/Star.h"
#include "scene/bodies/ico_mesh.h"

#include <vector>
#include <unordered_map>

enum class PlanetType {
    BODY,
    STAR,
    EARTH
};

const std::unordered_map<std::string, PlanetType> PLANET_TYPE_MAP = {
    {"body", PlanetType::BODY},
    {"star", PlanetType::STAR},
    {"earth", PlanetType::EARTH}
};

class PlanetSystem {
   private:
    std::vector<Body*> bodies;
    int selected_body = 0;

    Config& config;
    GPUMesh ico_mesh{Mesh{}};

   public:
    PlanetSystem(Config& config) : config(config) {
        try {
            Mesh ico_mesh_cpu =
                generate_ico_mesh(config.planets_ico_mesh_resolution);
            ico_mesh = GPUMesh(ico_mesh_cpu);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create icosahedron GPUMesh: " << e.what()
                      << std::endl;
            return;
        }

        // TODO: load bodies from config
        for (const auto& planet_info : config.planets) {
            switch (PLANET_TYPE_MAP.at(planet_info.type)) {
                case PlanetType::STAR:
                    bodies.push_back(new Star(
                        config, glm::vec3(0.0f), planet_info.radius, ico_mesh));
                    break;
                case PlanetType::EARTH:
                    bodies.push_back(new Earth(
                        config, glm::vec3(0.0f), planet_info.radius, ico_mesh));
                    break;
                case PlanetType::BODY:
                default:
                    bodies.push_back(new Body(
                        config, glm::vec3(0.0f), planet_info.radius, ico_mesh));
                    break;
            }
            if (planet_info.parent_id >= 0 &&
                planet_info.parent_id < (int)bodies.size()) {
                bodies.back()->set_orbit(
                    planet_info.orbit_direction, planet_info.orbit_small_r,
                    planet_info.orbit_large_r, planet_info.orbit_normal,
                    planet_info.orbit_period,
                    bodies[planet_info.parent_id]);
            }
        }

        for (auto& body : bodies) {
            body->setup();
        }
    }

    void imgui() {
        ImGui::Separator();
        ImGui::SliderInt("Selected Body", &selected_body, 0,
                         (int)bodies.size() - 1);
        bodies[selected_body]->imGuiControl();
    }

    void update(float delta_time) {
        for (Body* body : bodies) {
            body->update((float)delta_time);
        }
    }

    void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix,
              const glm::vec3& camera_position, float screen_height) {
        float time = (float)glfwGetTime();
        
        std::vector<glm::vec4> bodies_pos_rad;
        glm::vec4 sun_pos_rad(0.0f);
        for (Body* body : bodies) {
            glm::vec3 pos = body->getPosition();
            float radius = body->getRadius();
            if (dynamic_cast<Star*>(body) != nullptr) {
                sun_pos_rad = glm::vec4(pos, radius);
            }
            else {
                bodies_pos_rad.push_back(glm::vec4(pos, radius));
            }
        }
        
        for (Body* body : bodies) {
            body->shader.bind();
            glUniform1f(body->shader.getUniformLocation("screenHeight"),
                        screen_height);
            glUniform1f(body->shader.getUniformLocation("fov"),
                        glm::radians(config.camera_fov_degrees));
            glUniform1f(body->shader.getUniformLocation("time"), time);

            glUniform1i(body->shader.getUniformLocation("binary_system"), 0);
            glUniform4fv(body->shader.getUniformLocation("sunPosRad"), 1,
                glm::value_ptr(sun_pos_rad));
        
            glUniform1i(body->shader.getUniformLocation("num_bodies"),
                (GLint)bodies_pos_rad.size());
            glUniform4fv(body->shader.getUniformLocation("bodyPosRadii"),
                (GLint)bodies_pos_rad.size(), glm::value_ptr(bodies_pos_rad[0]));    

            body->draw(view_matrix, projection_matrix, camera_position);
        }
    }
};