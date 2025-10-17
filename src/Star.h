#pragma once
#include "Body.h"

class Star : public Body {
   public:
    Star(const glm::vec3& pos, float r, GPUMesh& icosahedron_mesh)
        : Body(pos, r, icosahedron_mesh) {}

    void setup() {
        ShaderBuilder starBuilder;
        starBuilder.addStage(GL_VERTEX_SHADER,
                            RESOURCE_ROOT "shaders/star_vert.glsl");
        starBuilder.addStage(GL_TESS_CONTROL_SHADER,
                            RESOURCE_ROOT "shaders/star_tesc.glsl");
        starBuilder.addStage(GL_TESS_EVALUATION_SHADER,
                            RESOURCE_ROOT "shaders/star_tese.glsl");
        starBuilder.addStage(GL_FRAGMENT_SHADER,
                            RESOURCE_ROOT "shaders/star_frag.glsl");
        shader = starBuilder.build();
    }

    void update(float deltaTime) {
        Body::update(deltaTime);
    }

    void imGuiControl() {
        Body::imGuiControl();
    }
    
    void draw() const {
        glUniform1f(shader.getUniformLocation("radius"), radius);
        glUniform1f(shader.getUniformLocation("test"), test);

        icosahedronMesh.drawPatches(shader);
    }
};