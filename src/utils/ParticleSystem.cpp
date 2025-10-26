#include "ParticleSystem.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <random>

ParticleSystem::ParticleSystem(int maxParticles)
    : _maxParticles(maxParticles),
    _lastUsedParticle(0),
    _particles(maxParticles),
    _posSizeData(maxParticles * 4),
    _colorData(maxParticles * 4)
{
    _aliveCount = 0;

    // Init particles as dead
    for (auto& p : _particles) {
        p.life = -1.0f;
        p.cameraDistance = -1.0f;
    }

    // Quad geometry (billboard)
    static const GLfloat quadVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vboBillboard);
    glBindBuffer(GL_ARRAY_BUFFER, _vboBillboard);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(0, 0);

    glGenBuffers(1, &_vboPositions);
    glBindBuffer(GL_ARRAY_BUFFER, _vboPositions);
    glBufferData(GL_ARRAY_BUFFER, _maxParticles * 4 * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribDivisor(1, 1);

    glGenBuffers(1, &_vboColors);
    glBindBuffer(GL_ARRAY_BUFFER, _vboColors);
    glBufferData(GL_ARRAY_BUFFER, _maxParticles * 4 * sizeof(GLubyte), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

ParticleSystem::~ParticleSystem() {
    glDeleteBuffers(1, &_vboBillboard);
    glDeleteBuffers(1, &_vboPositions);
    glDeleteBuffers(1, &_vboColors);
    glDeleteVertexArrays(1, &_vao);
}

int ParticleSystem::findUnusedParticle() {
    for (int i = _lastUsedParticle; i < _maxParticles; i++) {
        if (_particles[i].life < 0) {
            _lastUsedParticle = i;
            return i;
        }
    }
    for (int i = 0; i < _lastUsedParticle; i++) {
        if (_particles[i].life < 0) {
            _lastUsedParticle = i;
            return i;
        }
    }
    return 0;
}

void ParticleSystem::spawn(const glm::vec3& origin, const glm::vec3& speed, const glm::vec2& colorR, const glm::vec2& colorG, const glm::vec2& colorB, float coneAngle, float life, float lifeDeviation, float size, float sizeDeviation) {
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    int idx = findUnusedParticle();
    Particle& p = _particles[idx];
       
    p.life = life + dist01(rng) * lifeDeviation;
    p.pos = origin;

    float coneAngleRad = glm::radians(coneAngle);
    float cosTheta = 1.0f - dist01(rng) * (1.0f - cos(coneAngleRad));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = glm::two_pi<float>() * dist01(rng);

    glm::vec3 dir(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    p.speed = dir + speed;

    std::uniform_int_distribution<int> distR(colorR.x, colorR.y);
    std::uniform_int_distribution<int> distG(colorG.x, colorG.y);
    std::uniform_int_distribution<int> distB(colorB.x, colorB.y);

    p.r = static_cast<unsigned char>(distR(rng));
    p.g = static_cast<unsigned char>(distG(rng));
    p.b = static_cast<unsigned char>(distB(rng));
    p.a = 255;

    p.size = size + dist01(rng) * sizeDeviation;
    p.cameraDistance = -1.0f;
}

void ParticleSystem::update(const glm::mat4& model,float dt, const glm::vec3& camPos, const glm::vec3& speed, float lifeThreshold) {
    for (auto& p : _particles) {
        if (p.life > 0.0f) {
            p.life -= dt;
            if (p.life > 0.0f) {
                p.speed += speed * dt * 0.5f;
                p.pos += p.speed * dt;

                glm::vec3 worldPos = glm::vec3(model * glm::vec4(p.pos, 1.0f));
                p.cameraDistance = glm::length(worldPos - camPos);

                if (p.life < lifeThreshold) {
                    float lifeDiv = p.life / lifeThreshold;

                    p.a = static_cast<GLubyte>(glm::clamp(lifeDiv * 255.0f, 0.0f, 255.0f));
                }
            }
            else {
                p.cameraDistance = -1.0f;
            }
        }
    }

    std::sort(_particles.begin(), _particles.end());

    int count = 0;
    for (auto& p : _particles) {
        if (p.life > 0.0f) {
            _posSizeData[4 * count + 0] = p.pos.x;
            _posSizeData[4 * count + 1] = p.pos.y;
            _posSizeData[4 * count + 2] = p.pos.z;
            _posSizeData[4 * count + 3] = p.size;

            _colorData[4 * count + 0] = p.r;
            _colorData[4 * count + 1] = p.g;
            _colorData[4 * count + 2] = p.b;
            _colorData[4 * count + 3] = p.a;

            ++count;
        }
        else {
            break;
        }
    }

    _aliveCount = count;
}

void ParticleSystem::draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model, const Shader& shader) {
    glm::vec3 camRight(view[0][0], view[1][0], view[2][0]);
    glm::vec3 camUp(view[0][1], view[1][1], view[2][1]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    shader.bind();
    glUniformMatrix4fv(shader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(shader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform3fv(shader.getUniformLocation("CameraRight_worldspace"), 1, glm::value_ptr(camRight));
    glUniform3fv(shader.getUniformLocation("CameraUp_worldspace"), 1, glm::value_ptr(camUp));

    glBindVertexArray(_vao);

    // Only draw alive particles!
    int count = _aliveCount;

    if (count > 0) {
        // Upload position/size data
        glBindBuffer(GL_ARRAY_BUFFER, _vboPositions);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * 4 * sizeof(GLfloat), _posSizeData.data());

        // Upload color data
        glBindBuffer(GL_ARRAY_BUFFER, _vboColors);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * 4 * sizeof(GLubyte), _colorData.data());

        // Draw instanced quads
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
    }

    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
