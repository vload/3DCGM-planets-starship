#pragma once
#include "scene/bodies/Body.h"

class NullBody : public Body {
   public:
    NullBody(Config& config, const glm::vec3& pos, float r,
             GPUMesh& icosahedron_mesh)
        : Body(config, pos, r, icosahedron_mesh) {}

    void setup() {}
};