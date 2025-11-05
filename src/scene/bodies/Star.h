#pragma once
#include "scene/bodies/Body.h"

class Star : public Body {
   public:
    Star(Config& config, const glm::vec3& pos, float r,
         GPUMesh& icosahedron_mesh)
        : Body(config, pos, r, icosahedron_mesh) {
        // initialize star parameters from config
        noise_octaves = config.star_params.noise_octaves;
        noise_lacunarity = config.star_params.noise_lacunarity;
        noise_persistence = config.star_params.noise_persistence;
        warp_noise_scale = config.star_params.warp_noise_scale;
        noise_scale = config.star_params.noise_scale;
        animation_speed = config.star_params.animation_speed;
        temperature = config.default_star_temperature;
    }

    void setup() {
        ShaderBuilder starBuilder;
        starBuilder.addStage(GL_VERTEX_SHADER,
                             RESOURCE_ROOT "shaders/bodies/ico_vert.glsl");
        starBuilder.addStage(GL_TESS_CONTROL_SHADER,
                             RESOURCE_ROOT "shaders/bodies/ico_tesc.glsl");
        starBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                             RESOURCE_ROOT "shaders/bodies/ico_tese.glsl");
        starBuilder.addStage(GL_FRAGMENT_SHADER,
                             RESOURCE_ROOT "shaders/bodies/star_frag.glsl");
        shader = starBuilder.build();
    }

    void imGuiControl() {
        Body::imGuiControl();
        ImGui::Separator();
        ImGui::SliderFloat("Temperature (C)", &temperature, 3000.0f, 40000.0f);
    }

    void set_uniforms() {  // this assumes the shader is already bound
        Body::set_uniforms();
        glUniform1i(shader.getUniformLocation("OCTAVES"), noise_octaves);
        glUniform1f(shader.getUniformLocation("LACUNARITY"), noise_lacunarity);
        glUniform1f(shader.getUniformLocation("PERSISTENCE"),
                    noise_persistence);
        glUniform1f(shader.getUniformLocation("warp_noise_scale"),
                    warp_noise_scale);
        glUniform1f(shader.getUniformLocation("noise_scale"), noise_scale);
        glUniform1f(shader.getUniformLocation("animation_speed"),
                    animation_speed);
        glm::vec4 starColorIntensity = getStarColorIntensity();
        glUniform4fv(shader.getUniformLocation("star_color_intensity"), 1,
                     &starColorIntensity[0]);
    }

    // returns rgb and w = intensity
    glm::vec4 getStarColorIntensity() {
        // Define the colors
        const glm::vec4 COLOR_BLUE_WHITE =
            glm::vec4(0.56f, 0.68f, 0.8f, 100.0f);
        const glm::vec4 COLOR_WHITE = glm::vec4(0.9f, 0.9f, 0.9f, 80.0f);
        const glm::vec4 COLOR_YELLOW = glm::vec4(0.8f, 0.8f, 0.0f, 50.0f);
        const glm::vec4 COLOR_ORANGE = glm::vec4(0.8f, 0.4f, 0.0f, 20.0f);
        const glm::vec4 COLOR_RED = glm::vec4(0.8f, 0.0f, 0.0f, 10.0f);

        // Define the temperature key points
        const float TEMP_BLUE_WHITE = 40000.0f;
        const float TEMP_WHITE = 10000.0f;
        const float TEMP_YELLOW = 6000.0f;
        const float TEMP_ORANGE = 4500.0f;
        const float TEMP_RED = 3000.0f;

        if (temperature >= TEMP_BLUE_WHITE) {
            return COLOR_BLUE_WHITE;
        }
        if (temperature >= TEMP_WHITE) {
            float t =
                (temperature - TEMP_WHITE) / (TEMP_BLUE_WHITE - TEMP_WHITE);
            return glm::mix(COLOR_WHITE, COLOR_BLUE_WHITE, t);
        }
        if (temperature >= TEMP_YELLOW) {
            float t = (temperature - TEMP_YELLOW) / (TEMP_WHITE - TEMP_YELLOW);
            return glm::mix(COLOR_YELLOW, COLOR_WHITE, t);
        }
        if (temperature >= TEMP_ORANGE) {
            float t = (temperature - TEMP_ORANGE) / (TEMP_YELLOW - TEMP_ORANGE);
            return glm::mix(COLOR_ORANGE, COLOR_YELLOW, t);
        }
        if (temperature >= TEMP_RED) {
            float t = (temperature - TEMP_RED) / (TEMP_ORANGE - TEMP_RED);
            return glm::mix(COLOR_RED, COLOR_ORANGE, t);
        }

        return COLOR_RED;
    }

   protected:
    // star surface noise
    int noise_octaves = 5;
    float noise_lacunarity = 2.0f;
    float noise_persistence = 0.3f;
    float warp_noise_scale = 1.5f;
    float noise_scale = 20.0f;
    float animation_speed = 0.3f;
    float temperature = 5778.0f;  // sun-like
};