#pragma once
#include "scene/bodies/Body.h"

class Earth : public Body {
   public:
    // TODO: load earth params from config instead of insane imgui
    Earth(Config& config, const glm::vec3& pos, float r, GPUMesh& icosahedron_mesh)
        : Body(config, pos, r, icosahedron_mesh) {}

    void setup() {
        ShaderBuilder earthBuilder;
        earthBuilder.addStage(GL_VERTEX_SHADER,
                            RESOURCE_ROOT "shaders/ico_vert.glsl");
        earthBuilder.addStage(GL_TESS_CONTROL_SHADER,
                            RESOURCE_ROOT "shaders/ico_tesc.glsl");
        earthBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                            RESOURCE_ROOT "shaders/earth_tese.glsl");
        earthBuilder.addStage(GL_FRAGMENT_SHADER,
                            RESOURCE_ROOT "shaders/earth_frag.glsl");
        shader = earthBuilder.build();
    }

    void update(float deltaTime) {
        Body::update(deltaTime);
    }

    void imGuiControl() {
        Body::imGuiControl();
        ImGui::SliderFloat("Ocean Level", &ocean_level, -1.0f, 1.0f, "%.2f");
        ImGui::SliderInt("Shape Noise Octaves", &shape_noise_octaves, 1, 10);
        ImGui::SliderFloat("Shape Noise Lacunarity", &shape_noise_lacunarity, 1.0f, 4.0f, "%.2f");
        ImGui::SliderFloat("Shape Noise Persistence", &shape_noise_persistence, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Shape Noise Base Frequency", &shape_noise_base_frequency, 1.0f, 10.0f, "%.2f");
        ImGui::SliderFloat("Shape Noise Pseudo Seed", &shape_noise_pseudo_seed, 0.0f, 10000.0f, "%.2f");
        ImGui::SliderFloat("Shape Noise Scale", &shape_noise_scale, 0.0f, 1.0f, "%.2f");
        ImGui::Separator();
        ImGui::SliderInt("Water Noise Octaves", &water_noise_octaves, 1, 10);
        ImGui::SliderFloat("Water Noise Lacunarity", &water_noise_lacunarity, 1.0f, 4.0f, "%.2f");
        ImGui::SliderFloat("Water Noise Persistence", &water_noise_persistence, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Ocean Scale", &ocean_scale, 1.0f, 100.0f, "%.2f");
        ImGui::SliderFloat("Ocean Speed", &ocean_speed, 0.0f, 5.0f, "%.2f");
        ImGui::Separator();
        ImGui::SliderFloat("Water Ka", &waterKa, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Water Kd", &waterKd, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Water Ks", &waterKs, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Water Shininess", &waterShininess, 1.0f, 256.0f, "%.2f");
    }

    virtual void set_uniforms() {  // this assumes the shader is already bound
        Body::set_uniforms();
        glUniform1f(shader.getUniformLocation("ocean_level"), ocean_level);
        glUniform1i(shader.getUniformLocation("OCTAVES_shape"),
                    shape_noise_octaves);
        glUniform1f(shader.getUniformLocation("LACUNARITY_shape"),
                    shape_noise_lacunarity);
        glUniform1f(shader.getUniformLocation("PERSISTENCE_shape"), shape_noise_persistence);
        glUniform1f(shader.getUniformLocation("shape_noise_base_frequency"),
                    shape_noise_base_frequency);
        glUniform1f(shader.getUniformLocation("shape_noise_pseudo_seed"),
                    shape_noise_pseudo_seed);
        glUniform1f(shader.getUniformLocation("shape_noise_scale"),
                    shape_noise_scale);

        glUniform1i(shader.getUniformLocation("OCTAVES_water"),
                    water_noise_octaves);
        glUniform1f(shader.getUniformLocation("LACUNARITY_water"),
                    water_noise_lacunarity);
        glUniform1f(shader.getUniformLocation("PERSISTENCE_water"), water_noise_persistence);
        glUniform1f(shader.getUniformLocation("ocean_scale"),
                    ocean_scale);
        glUniform1f(shader.getUniformLocation("ocean_speed"),
                    ocean_speed);
        glUniform1f(shader.getUniformLocation("waterKa"), waterKa);
        glUniform1f(shader.getUniformLocation("waterKd"), waterKd);
        glUniform1f(shader.getUniformLocation("waterKs"), waterKs);
        glUniform1f(shader.getUniformLocation("waterShininess"), waterShininess);
    }

   protected:
    // Earth shape parameters
    int shape_noise_octaves = 5;
    float shape_noise_lacunarity = 2.0f;
    float shape_noise_persistence = 0.45f;
    float shape_noise_base_frequency = 5.0f;
    float ocean_level = 0.0f;
    float shape_noise_pseudo_seed = 100.0f;
    float shape_noise_scale = 0.1f;
    // Earth water parameters
    int water_noise_octaves = 5;
    float water_noise_lacunarity = 2.0f;
    float water_noise_persistence = 0.45f;
    float ocean_scale = 10.0f;
    float ocean_speed = 0.3f;
    float waterKa = 0.2f;
    float waterKd = 0.8f;
    float waterKs = 0.9f;
    float waterShininess = 128.0f;
};