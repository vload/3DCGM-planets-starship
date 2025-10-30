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

    void spawn(const glm::mat4& model, const glm::vec3& origin);
    void update(const glm::mat4& model, float dt, const glm::vec3& camPos);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

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

    Shader shader;

    std::vector<GLfloat> _posSizeData;
    std::vector<GLubyte> _colorData;

    glm::vec3 speedInitParticle = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec2 colorR = glm::vec2(233.0f, 255.f);
    glm::vec2 colorG = glm::vec2(165.0f, 255.f);
    glm::vec2 colorB = glm::vec2(0.0f, 0.0f);
    
    float life = 1.0f;
    float lifeDeviation = 0.5f;
    float lifeThreshold = 0.5f;
    float size = 0.06f;
    float sizeDeviation = 0.06f;

    float spawnRadius = 0.2f;    
    float coneAngleDeviation = 5.0f;
    float coneAngle = 30.0f;
    float velocitySpread = 0.2f;
};
