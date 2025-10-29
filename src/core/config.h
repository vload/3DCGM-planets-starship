#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <toml/toml.hpp>

#include <optional>
#include <iostream>
#include <string>

class Config {
public:
    struct PlanetInfo {
        std::string type;
        float radius;
        int parent_id;

        // Orbit parameters
        glm::vec3 orbit_direction;
        float orbit_small_r;
        float orbit_large_r;
        glm::vec3 orbit_normal;
        float orbit_period;
    };

    std::string window_title;
    int window_initial_width;
    int window_initial_height;

    float camera_fov_degrees;
    glm::vec3 freecam_initial_position;
    glm::vec3 freecam_initial_forward;
    float freecam_move_speed;
    float freecam_look_speed;

    int planets_ico_mesh_resolution;
    std::vector<PlanetInfo> planets;

    bool enable_eclipse_shadows;
    bool enable_shadow_mapping_planets;
    int shadow_map_size;
    
    void load_config(const char* path) {
        // Load configuration from toml file at 'path'
        toml::table data = toml::parse_file(path);
        window_title = data["window"]["title"].value_or("Default Title");
        window_initial_width = data["window"]["initial_width"].value_or(1280);
        window_initial_height = data["window"]["initial_height"].value_or(720);

        camera_fov_degrees = data["camera"]["fov"].value_or(80.0f);
        freecam_initial_position =
            tomlArrayToVec3(
                data["camera"]["freecam"]["initial_position"].as_array())
                .value_or(glm::vec3(5.0f, 0.0f, 0.0f));
        freecam_initial_forward =
            tomlArrayToVec3(
                data["camera"]["freecam"]["initial_forward"].as_array())
                .value_or(glm::vec3(-1.0f, 0.0f, 0.0f));
        freecam_move_speed = data["camera"]["freecam"]["move_speed"].value_or(0.5f);
        freecam_look_speed = data["camera"]["freecam"]["look_speed"].value_or(0.035f);

        planets_ico_mesh_resolution = data["planets"]["ico_mesh_resolution"].value_or(5);

        // Get the underlying array object for planets_info
        if (toml::array* planets_array = data["planets"]["planets_info"].as_array()) {
            planets_array->for_each([&](auto&& planet) {
                toml::array* planet_arr = planet.as_array();
                if (!planet_arr) {
                    std::cerr << "Error: Expected array for planet info, got "
                              << planet.type() << std::endl;
                    return;
                }

                PlanetInfo info;
                info.type = (*planet_arr)[0].value_or("body");
                info.radius = (*planet_arr)[1].value_or(1.0f);
                info.parent_id = (*planet_arr)[2].value_or(-1);
                info.orbit_direction = tomlArrayToVec3((*planet_arr)[3].as_array()).value_or(glm::vec3(0.0f));
                info.orbit_small_r = (*planet_arr)[4].value_or(0.0f);
                info.orbit_large_r = (*planet_arr)[5].value_or(0.0f);
                info.orbit_normal = tomlArrayToVec3((*planet_arr)[6].as_array()).value_or(glm::vec3(0.0f));
                info.orbit_period = (*planet_arr)[7].value_or(0.0f);
                planets.push_back(info);
            });
        }

        enable_eclipse_shadows = data["shadows"]["enable_eclipse_shadows"].value_or(false);
        enable_shadow_mapping_planets = data["shadows"]["enable_shaddow_mapping_planets"].value_or(false);
        shadow_map_size = data["shadows"]["shadow_map_size"].value_or(2048);
    }

private:
    std::optional<glm::vec3> tomlArrayToVec3(const toml::array* array) {
        glm::vec3 output{};

        if (array) {
            int i = 0;
            array->for_each([&](auto&& elem) {
                if (elem.is_number()) {
                    if (i > 2) return;
                    output[i] =
                        static_cast<float>(elem.as_floating_point()->get());
                    i += 1;
                } else {
                    std::cerr << "Error: Expected a number in array, got "
                            << elem.type() << std::endl;
                    return;
                }
            });
        }

        return output;
    }
};