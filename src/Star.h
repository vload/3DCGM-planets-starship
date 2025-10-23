#pragma once
#include "Body.h"

class Star : public Body {
   public:
    Star(const glm::vec3& pos, float r, GPUMesh& icosahedron_mesh)
        : Body(pos, r, icosahedron_mesh) {}

    void setup() {
        ShaderBuilder starBuilder;
        starBuilder.addStage(GL_VERTEX_SHADER,
                            RESOURCE_ROOT "shaders/ico_vert.glsl");
        starBuilder.addStage(GL_TESS_CONTROL_SHADER,
                            RESOURCE_ROOT "shaders/ico_tesc.glsl");
        starBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                            RESOURCE_ROOT "shaders/ico_tese.glsl");
        starBuilder.addStage(GL_FRAGMENT_SHADER,
                            RESOURCE_ROOT "shaders/star_frag.glsl");
        shader = starBuilder.build();
    }

    void update(float deltaTime) {
        Body::update(deltaTime);
    }

    void imGuiControl() {
        Body::imGuiControl();
        ImGui::Separator();
        ImGui::Text("Star Surface Noise Controls");
        ImGui::SliderInt("Noise Octaves", &noise_octaves, 1, 10);
        ImGui::SliderFloat("Noise Lacunarity", &noise_lacunarity, 1.0f, 4.0f);
        ImGui::SliderFloat("Noise Persistence", &noise_persistence, 0.1f, 1.0f);
        ImGui::SliderFloat("Warp Noise Scale", &warp_noise_scale, 0.1f, 5.0f);
        ImGui::SliderFloat("Noise Scale", &noise_scale, 1.0f, 100.0f);
        ImGui::SliderFloat("Animation Speed", &animation_speed, 0.1f, 1.0f);
    }

    void set_uniforms() { // this assumes the shader is already bound
        Body::set_uniforms();
        glUniform1i(shader.getUniformLocation("OCTAVES"), noise_octaves);
        glUniform1f(shader.getUniformLocation("LACUNARITY"), noise_lacunarity);
        glUniform1f(shader.getUniformLocation("PERSISTENCE"), noise_persistence);
        glUniform1f(shader.getUniformLocation("warp_noise_scale"), warp_noise_scale);
        glUniform1f(shader.getUniformLocation("noise_scale"), noise_scale);
        glUniform1f(shader.getUniformLocation("animation_speed"), animation_speed);
    }

   protected:
    // star surface noise
    int noise_octaves = 5;
    float noise_lacunarity = 2.0f;
    float noise_persistence = 0.3f;
    float warp_noise_scale = 1.5f;
    float noise_scale = 20.0f;
    float animation_speed = 0.3f;
};