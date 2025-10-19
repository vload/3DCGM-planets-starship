#pragma once
#include "Body.h"

class Earth : public Body {
   public:
    Earth(const glm::vec3& pos, float r, GPUMesh& icosahedron_mesh)
        : Body(pos, r, icosahedron_mesh) {}

    void setup() {
        ShaderBuilder earthBuilder;
        earthBuilder.addStage(GL_VERTEX_SHADER,
                            RESOURCE_ROOT "shaders/earth_vert.glsl");
        earthBuilder.addStage(GL_TESS_CONTROL_SHADER,
                            RESOURCE_ROOT "shaders/earth_tesc.glsl");
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
        ImGui::SliderFloat("Ocean Depth", &ocean_depth, 0.1f, 10.0f, "%.2f");
    }
    
    void draw() const {
        glUniform1f(shader.getUniformLocation("radius"), radius);
        glUniform1f(shader.getUniformLocation("test"), test);
        glUniform1f(shader.getUniformLocation("ocean_depth"), ocean_depth);
        
        icosahedronMesh.drawPatches(shader);
    }
   protected:
    float ocean_depth = 2.0f;
};