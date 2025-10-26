#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <glad/glad.h>
#include <framework/shader.h>

struct Particle {
    glm::vec3 pos;
    glm::vec3 speed;
    unsigned char r, g, b, a;
    float size;
    float life;
    float cameraDistance;

    bool operator<(const Particle& that) const {
        return cameraDistance > that.cameraDistance;
    }
};

struct LightParticle {
    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 color;
    float angle;
    float thresholdLight;
    float intensity;
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 100000);
    ~ParticleSystem();

    void spawn(const glm::vec3& origin, const glm::vec3& speed, const glm::vec2& colorR, const glm::vec2& colorG, const glm::vec2& colorB, float coneAngle, float life, float lifeDeviation, float size, float sizeDeviation);
    void update(const glm::mat4& model, float dt, const glm::vec3& camPos, const glm::vec3& speed, float lifeThreshold);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const Shader& shader);

private:
    int findUnusedParticle();

    int _maxParticles;
    int _lastUsedParticle;
    int _aliveCount;
    std::vector<Particle> _particles;

    GLuint _vao = 0;
    GLuint _vboBillboard = 0;
    GLuint _vboPositions = 0;
    GLuint _vboColors = 0;

    std::vector<GLfloat> _posSizeData;
    std::vector<GLubyte> _colorData;
};
