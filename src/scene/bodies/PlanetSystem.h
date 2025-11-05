#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui/imgui.h>

#include <glm/glm.hpp>
DISABLE_WARNINGS_POP()

#include <unordered_map>
#include <vector>

#include "core/config.h"
#include "core/mesh.h"
#include "scene/bodies/Body.h"
#include "scene/bodies/Earth.h"
#include "scene/bodies/NullBody.h"
#include "scene/bodies/Star.h"
#include "scene/bodies/ico_mesh.h"

enum class PlanetType { BODY, STAR, EARTH, NULL_BODY };

const std::unordered_map<std::string, PlanetType> PLANET_TYPE_MAP = {
    {"null", PlanetType::NULL_BODY},
    {"body", PlanetType::BODY},
    {"star", PlanetType::STAR},
    {"earth", PlanetType::EARTH}};

class PlanetSystem {
   private:
    std::vector<Body*> bodies;
    int selected_body = 0;

    float exposure = 0.2f;
    float gamma = 1.4f;
    float L_target = 0.4f;

    Config& config;
    GPUMesh ico_mesh{Mesh{}};

    float target_body_tessellation_triangle_height = config.target_triangle_height;
    bool enable_body_tessellation = config.enable_body_tesellation;
    bool enable_eclipse = true;
    bool enable_shadowmapping = true;
    int PCF_kernel_radius = 1;
    bool enable_PCF = true;
    bool is_binary_system = false;
    bool planet_water_normals_enabled = true;
    bool planet_surface_normals_enabled = true;
    bool sun_texture_enabled = config.enable_star_texture;
    bool use_star_color = config.use_star_color;

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

        enable_eclipse = config.enable_eclipse_shadows;
        is_binary_system = config.is_binary_system;
        L_target = config.target_gray_brightness;
        gamma = config.gamma;

        for (const auto& planet_info : config.planets) {
            switch (PLANET_TYPE_MAP.at(planet_info.type)) {
                case PlanetType::STAR:
                    bodies.push_back(new Star(config, glm::vec3(0.0f),
                                              planet_info.radius, ico_mesh));
                    break;
                case PlanetType::EARTH:
                    bodies.push_back(new Earth(config, glm::vec3(0.0f),
                                               planet_info.radius, ico_mesh));
                    break;
                case PlanetType::NULL_BODY:
                    bodies.push_back(new NullBody(
                        config, glm::vec3(0.0f), planet_info.radius, ico_mesh));
                    break;
                case PlanetType::BODY:
                default:
                    bodies.push_back(new Body(config, glm::vec3(0.0f),
                                              planet_info.radius, ico_mesh));
                    break;
            }
            if (planet_info.parent_id >= 0 &&
                planet_info.parent_id < (int)bodies.size()) {
                bodies.back()->set_orbit(
                    planet_info.orbit_direction, planet_info.orbit_small_r,
                    planet_info.orbit_large_r, planet_info.orbit_normal,
                    planet_info.orbit_period, bodies[planet_info.parent_id]);
            }
        }

        for (auto& body : bodies) {
            body->setup();
        }
    }

    void imgui() {
        ImGui::Separator();
        ImGui::DragFloat("target gray brightness (for exposure)", &L_target, 0.01f, 0.01f, 1.0f);
        float l_total = 0.0;
        int count = 0;
        for(Body* body : bodies) {
            Star* star = dynamic_cast<Star*>(body);
            if(star != nullptr) {
                l_total += star->getStarColorIntensity().w;
                count ++;

                if (count >= (is_binary_system ? 2 : 1)) break;
            }
        }
        exposure = L_target / l_total / (1.0f - L_target);

        ImGui::DragFloat("Gamma (brightness)", &gamma, 0.1f, 1.0f, 5.0f);
        ImGui::Checkbox("Body Tessellation", &enable_body_tessellation);
        ImGui::DragFloat("Target Body Tessellation Triangle Height",
                         &target_body_tessellation_triangle_height, 0.1f, 1.0f,
                         20.0f);
        ImGui::Checkbox("Enable Eclipse", &enable_eclipse);
        ImGui::Checkbox("Enable Shadowmapping", &enable_shadowmapping);
        ImGui::Checkbox("Enable PCF", &enable_PCF);
        ImGui::DragInt("PCF Kernel Radius", &PCF_kernel_radius, 1, 0, 10);
        ImGui::Checkbox("Enable Planet Water Normals", &planet_water_normals_enabled);
        ImGui::Checkbox("Enable Planet Surface Normals", &planet_surface_normals_enabled);
        ImGui::Checkbox("Enable Star Texture / Animation", &sun_texture_enabled);
        ImGui::Checkbox("Use Star Color for Lighting", &use_star_color);
        ImGui::SliderInt("Selected Body", &selected_body, 0,
            (int)bodies.size() - 1);
        bodies[selected_body]->imGuiControl();

        ImGui::Separator();
        ImGui::Text("Add body orbiting selected body:");
        if (ImGui::Button("Add Earth")) {
            bodies.push_back(
                new Earth(config, glm::vec3(0.0f), 1.0f, ico_mesh));
            bodies.back()->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 7.0f, 7.0f,
                                     glm::vec3(0.0f, 1.0f, 0.0f), 10.0f,
                                     bodies[selected_body]);
            bodies.back()->setup();
            selected_body = (int)bodies.size() - 1;
        }
        if (ImGui::Button("Add Default Body")) {
            bodies.push_back(new Body(config, glm::vec3(0.0f), 1.0f, ico_mesh));
            bodies.back()->set_orbit(glm::vec3(1.0f, 0.0f, 0.0f), 7.0f, 7.0f,
                                     glm::vec3(0.0f, 1.0f, 0.0f), 10.0f,
                                     bodies[selected_body]);
            bodies.back()->setup();
            selected_body = (int)bodies.size() - 1;
        }
    }
    void update(float delta_time) {
        auto [binary_star_pos1, binary_star_pos2] = getBinaryStarPositions();
        for (Body* body : bodies) {
            body->update(delta_time, binary_star_pos1, binary_star_pos2);
        }
    }

    std::pair<glm::vec3, glm::vec3> getBinaryStarPositions() const {
         if (!is_binary_system) {
            for (Body* body : bodies) {
                // assume sun is bodies[0]
                return {bodies[0]->getPosition(), glm::vec3(0.0f)};
            }
        }

        // max two stars are light sources
        std::vector<glm::vec3> star_pos;
        for (Body* body : bodies) {
            glm::vec3 pos = body->getPosition();
            Star* star = dynamic_cast<Star*>(body);
            if (star != nullptr) {
                star_pos.push_back(pos);
            }
            if(star_pos.size() >=2) break;
        }

        if (star_pos.size() < 2)
            throw std::runtime_error(
                "Binary system enabled but number of stars < 2");
        
        return {star_pos[0], star_pos[1]};
    }

    auto getEclipseShadomappingPCFKernelExposureGammaSettings() const {
        return std::make_tuple(enable_eclipse, enable_shadowmapping,
                               enable_PCF, PCF_kernel_radius, exposure, gamma);
    }

    auto getBodiesAndSunsInfoForShader() const {
        std::vector<glm::vec4> bodies_pos_rad;
        std::vector<glm::vec4> sun_pos_rads;
        std::vector<glm::vec4> sun_col_ints;
        for (Body* body : bodies) {
            if (dynamic_cast<NullBody*>(body) != nullptr) {
                // skip NullBody in rendering
                continue;
            }

            glm::vec3 pos = body->getPosition();
            float radius = body->getRadius();
            Star* star = dynamic_cast<Star*>(body);
            if (star != nullptr) {
                // only first stars are light sources
                if (sun_pos_rads.size() < (is_binary_system ? 2 : 1)) {
                    sun_pos_rads.push_back(glm::vec4(pos, radius));
                    if (use_star_color) {
                        sun_col_ints.push_back(star->getStarColorIntensity());
                    } else {
                        sun_col_ints.push_back(glm::vec4(
                            glm::vec3(1.0), star->getStarColorIntensity().w));
                    }
                }
            }

            bodies_pos_rad.push_back(glm::vec4(pos, radius));
        }

        return std::make_tuple(bodies_pos_rad, sun_pos_rads, sun_col_ints);
    }

    void draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix,
              const glm::vec3& camera_position, float screen_height,
              void (*reset_opengl_state)()){
                
        float time = (float)glfwGetTime();

        auto [bodies_pos_rad, sun_pos_rads,
              sun_col_ints] = getBodiesAndSunsInfoForShader();
        // shadow map pass
        for (Body* body : bodies) {
            if (dynamic_cast<NullBody*>(body) != nullptr) {
                // skip NullBody in rendering
                continue;
            }
            if (body->needs_shadow_map()) {
                reset_opengl_state();
                body->shader.bind();
                glUniform1f(body->shader.getUniformLocation("screenHeight"),
                            screen_height);
                glUniform1f(body->shader.getUniformLocation("fov"),
                            glm::radians(config.camera_fov_degrees));
                glUniform1f(body->shader.getUniformLocation("time"), time);

                glUniform1i(body->shader.getUniformLocation("binary_system"),
                            is_binary_system);
                glUniform4fv(body->shader.getUniformLocation("sunPosRadii"),
                             (GLint)sun_pos_rads.size(),
                             glm::value_ptr(sun_pos_rads[0]));
                glUniform4fv(body->shader.getUniformLocation("sunColInt"),
                             (GLint)sun_col_ints.size(),
                             glm::value_ptr(sun_col_ints[0]));

                glUniform1i(body->shader.getUniformLocation("num_bodies"),
                            (GLint)bodies_pos_rad.size());
                glUniform4fv(body->shader.getUniformLocation("bodyPosRadii"),
                             (GLint)bodies_pos_rad.size(),
                             glm::value_ptr(bodies_pos_rad[0]));
                glUniform1i(body->shader.getUniformLocation("tessellate"),
                            enable_body_tessellation);
                glUniform1f(body->shader.getUniformLocation("targetPixelSize"),
                            target_body_tessellation_triangle_height);

                body->draw_depth();
            }
        }

        // regular draw pass
        for (Body* body : bodies) {
            if (dynamic_cast<NullBody*>(body) != nullptr) {
                // skip NullBody in rendering
                continue;
            }
            reset_opengl_state();
            body->shader.bind();
            glUniform1f(body->shader.getUniformLocation("screenHeight"),
                        screen_height);
            glUniform1f(body->shader.getUniformLocation("fov"),
                        glm::radians(config.camera_fov_degrees));
            glUniform1f(body->shader.getUniformLocation("time"), time);

            glUniform1i(body->shader.getUniformLocation("binary_system"),
                        is_binary_system);
            glUniform4fv(body->shader.getUniformLocation("sunPosRadii"),
                         (GLint)sun_pos_rads.size(),
                         glm::value_ptr(sun_pos_rads[0]));
            glUniform4fv(body->shader.getUniformLocation("sunColInt"),
                         (GLint)sun_col_ints.size(),
                         glm::value_ptr(sun_col_ints[0]));
            glUniform1i(body->shader.getUniformLocation(
                            "enable_ocean_normal_calculation"),
                        planet_water_normals_enabled ? 1 : 0);
            glUniform1i(body->shader.getUniformLocation(
                            "enable_surface_normal_calculation"),
                        planet_surface_normals_enabled ? 1 : 0);

            glUniform1f(body->shader.getUniformLocation("exposure"), exposure);
            glUniform1f(body->shader.getUniformLocation("gamma"), gamma);

            glUniform1i(body->shader.getUniformLocation("num_bodies"),
                        (GLint)bodies_pos_rad.size());
            glUniform4fv(body->shader.getUniformLocation("bodyPosRadii"),
                         (GLint)bodies_pos_rad.size(),
                         glm::value_ptr(bodies_pos_rad[0]));
            glUniform1i(body->shader.getUniformLocation("tessellate"),
                        enable_body_tessellation);
            glUniform1f(body->shader.getUniformLocation("targetPixelSize"),
                        target_body_tessellation_triangle_height);
            glUniform1i(body->shader.getUniformLocation("enable_eclipse"),
                        enable_eclipse ? 1 : 0);
            glUniform1i(body->shader.getUniformLocation("enable_shadowmapping"),
                        enable_shadowmapping ? 1 : 0);
            glUniform1i(body->shader.getUniformLocation("PCF_kernel_radius"),
                        PCF_kernel_radius);
            glUniform1i(body->shader.getUniformLocation("enable_PCF"),
                        enable_PCF ? 1 : 0);
            glUniform1i(body->shader.getUniformLocation("sun_texture_enabled"),
                        sun_texture_enabled ? 1 : 0);

            body->draw(view_matrix, projection_matrix, camera_position);
        }
    }
};