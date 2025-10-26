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
    std::string window_title;
    int window_initial_width;
    int window_initial_height;

    float camera_fov_degrees;
    glm::vec3 freecam_initial_position;
    glm::vec3 freecam_initial_forward;
    float freecam_move_speed;
    float freecam_look_speed;

    int planets_ico_mesh_resolution;
    
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