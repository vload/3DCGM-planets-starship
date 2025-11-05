#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <toml/toml.hpp>

#include <optional>
#include <iostream>
#include <string>
#include <vector>

class Config {
public:
    struct BattlecruiserPathInfo {
        float start_point_factor;
        float end_point_factor;

        std::vector<glm::vec3> origin_point_list;
        std::vector<glm::vec3> previous_point_list;
        std::vector<float> next_point_factor_list;
    };

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

    // Earth-specific parameters (shape + water)
    struct EarthParams {
        // Earth shape parameters
        int shape_noise_octaves = 5;
        float shape_noise_lacunarity = 2.0f;
        float shape_noise_persistence = 0.45f;
        float shape_noise_base_frequency = 1.2f;
        float ocean_level = 0.0f;
        float shape_noise_pseudo_seed = 100.0f;
        float shape_noise_scale = 0.25f;
        float surface_ka = 0.1f;
        float surface_kd = 0.9f;

        // Earth water parameters
        int water_noise_octaves = 5;
        float water_noise_lacunarity = 2.0f;
        float water_noise_persistence = 0.45f;
        float ocean_scale = 10.0f;
        float ocean_speed = 0.3f;
        float waterKa = 0.1f;
        float waterKd = 0.9f;
        float waterKs = 0.9f;
        float waterShininess = 128.0f;
        float ocean_normal_amplitude = 0.01f;
    };

    std::string window_title;
    int window_initial_width;
    int window_initial_height;

    float camera_fov_degrees;
    glm::vec3 freecam_initial_position;
    glm::vec3 freecam_initial_forward;
    float freecam_move_speed;
    float freecam_look_speed;

    float target_gray_brightness;
    float gamma;

    int planets_ico_mesh_resolution;
    std::vector<PlanetInfo> planets;

    bool enable_eclipse_shadows;
    bool enable_shadow_mapping_planets;
    int shadow_map_size;
    bool is_binary_system;
    
    glm::vec3 body_fallback_color;

    BattlecruiserPathInfo battlecruiser_path_info;
    EarthParams earth_params;

    // Star-specific parameters (surface noise / animation)
    struct StarParams {
        int noise_octaves = 5;
        float noise_lacunarity = 2.0f;
        float noise_persistence = 0.3f;
        float warp_noise_scale = 1.5f;
        float noise_scale = 20.0f;
        float animation_speed = 0.3f;
    };

    StarParams star_params;
    
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
        target_gray_brightness = data["camera"]["target_gray_brightness"].value_or(0.4f);
        gamma = data["camera"]["gamma"].value_or(1.3f);

        body_fallback_color =
            tomlArrayToVec3(
                data["planets"]["fallback_color"].as_array())
                .value_or(glm::vec3(0.3f, 0.3f, 1.0f));

        is_binary_system = data["planets"]["binary_system"].value_or(false);

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

        // Get the object for battlecruiser_path info
        if (toml::table* bc_table = data["battlecruiser"]["path"].as_table()) {
            battlecruiser_path_info.start_point_factor =
                bc_table->get("start_point_factor")->value_or(1.0f);
            battlecruiser_path_info.end_point_factor =
                bc_table->get("end_point_factor")->value_or(1.0f);

            if (toml::array* path_list = bc_table->get("path_points_list")->as_array()) {
                path_list->for_each([&](auto&& elem) {
                    toml::array* triple = elem.as_array();
                    if (!triple || triple->size() != 3) {
                        std::cerr << "Error: Expected [vec3, vec3, float] triple in path_points_list\n";
                        return;
                    }

                    glm::vec3 previous = tomlArrayToVec3((*triple)[0].as_array()).value_or(glm::vec3(0.0f));
                    glm::vec3 origin  = tomlArrayToVec3((*triple)[1].as_array()).value_or(glm::vec3(0.0f));
                    float next_factor    = (*triple)[2].value_or(0.0f);

                    battlecruiser_path_info.previous_point_list.push_back(previous);
                    battlecruiser_path_info.origin_point_list.push_back(origin);
                    battlecruiser_path_info.next_point_factor_list.push_back(next_factor);
                });
            }
        }

        // Read earth-specific params (global defaults for "earth" type)
        if (toml::table* earth_table = data["planets"]["earth"].as_table()) {
            earth_params.shape_noise_octaves =
                earth_table->get("shape_noise_octaves")->value_or(5);
            earth_params.shape_noise_lacunarity =
                earth_table->get("shape_noise_lacunarity")->value_or(2.0f);
            earth_params.shape_noise_persistence =
                earth_table->get("shape_noise_persistence")->value_or(0.45f);
            earth_params.shape_noise_base_frequency =
                earth_table->get("shape_noise_base_frequency")->value_or(1.2f);
            earth_params.ocean_level =
                earth_table->get("ocean_level")->value_or(0.0f);
            earth_params.shape_noise_pseudo_seed =
                earth_table->get("shape_noise_pseudo_seed")->value_or(100.0f);
            earth_params.shape_noise_scale =
                earth_table->get("shape_noise_scale")->value_or(0.25f);
            earth_params.surface_ka =
                earth_table->get("surface_ka")->value_or(0.1f);
            earth_params.surface_kd =
                earth_table->get("surface_kd")->value_or(0.9f);

            earth_params.water_noise_octaves =
                earth_table->get("water_noise_octaves")->value_or(5);
            earth_params.water_noise_lacunarity =
                earth_table->get("water_noise_lacunarity")->value_or(2.0f);
            earth_params.water_noise_persistence =
                earth_table->get("water_noise_persistence")->value_or(0.45f);
            earth_params.ocean_scale =
                earth_table->get("ocean_scale")->value_or(10.0f);
            earth_params.ocean_speed =
                earth_table->get("ocean_speed")->value_or(0.3f);
            earth_params.waterKa =
                earth_table->get("waterKa")->value_or(0.1f);
            earth_params.waterKd =
                earth_table->get("waterKd")->value_or(0.9f);
            earth_params.waterKs =
                earth_table->get("waterKs")->value_or(0.9f);
            earth_params.waterShininess =
                earth_table->get("waterShininess")->value_or(128.0f);
            earth_params.ocean_normal_amplitude =
                earth_table->get("ocean_normal_amplitude")->value_or(0.01f);
        }

        enable_eclipse_shadows = data["shadows"]["enable_eclipse_shadows"].value_or(false);
        enable_shadow_mapping_planets = data["shadows"]["enable_shaddow_mapping_planets"].value_or(false);
        shadow_map_size = data["shadows"]["shadow_map_size"].value_or(2048);

        // Read star-specific params (global defaults for "star" type)
        if (toml::table* star_table = data["planets"]["star"].as_table()) {
            star_params.noise_octaves =
                star_table->get("noise_octaves")->value_or(5);
            star_params.noise_lacunarity =
                star_table->get("noise_lacunarity")->value_or(2.0f);
            star_params.noise_persistence =
                star_table->get("noise_persistence")->value_or(0.3f);
            star_params.warp_noise_scale =
                star_table->get("warp_noise_scale")->value_or(1.5f);
            star_params.noise_scale =
                star_table->get("noise_scale")->value_or(20.0f);
            star_params.animation_speed =
                star_table->get("animation_speed")->value_or(0.3f);
        }
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