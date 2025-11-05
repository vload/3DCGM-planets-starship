#include "Minimap.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <random>
#include <imgui/imgui.h>

Minimap::Minimap() {
    float quadVertices[] = {
        // positions    // texCoords (used as UVs)
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,   0.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    initBuffers();
}

Minimap::~Minimap() {
    glDeleteFramebuffers(1, &minimapFrameBuffer);
    glDeleteTextures(1, &minimapColorTexture);
    glDeleteRenderbuffers(1, &minimapDepthBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Minimap::initBuffers() {
    glGenFramebuffers(1, &minimapFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, minimapFrameBuffer);

    // Color texture
    glGenTextures(1, &minimapColorTexture);
    glBindTexture(GL_TEXTURE_2D, minimapColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapColorTexture, 0);

    // Depth buffer
    glGenRenderbuffers(1, &minimapDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, minimapDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, minimapDepthBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    minimapShader = ShaderBuilder()
        .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT
                  "shaders/minimap_vert.glsl")
        .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
                  "shaders/minimap_frag.glsl")
        .build();

    minimapScreenShader = ShaderBuilder()
        .addStage(GL_VERTEX_SHADER, RESOURCE_ROOT "shaders/minimap_screen_vert.glsl")
        .addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT "shaders/minimap_screen_frag.glsl")
        .build();
}

void Minimap::draw(const std::vector<Body*>& bodies, const std::vector<glm::vec3>& positionPoints, glm::vec3 positionBattleCruiser) {
    std::vector<Object> planets = convertBodyToObjects(bodies);

    glBindFramebuffer(GL_FRAMEBUFFER, minimapFrameBuffer);
    glViewport(0, 0, 512, 512);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    float maxDistance = 100.0f;

    glm::mat4 projection = glm::ortho(-maxDistance, maxDistance,
                                -maxDistance, maxDistance,
                                -1.0f, 1.0f);
    minimapShader.bind();

    glUniformMatrix4fv(minimapShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);

    std::vector<Object> points = convertBezierPointsToObjects(positionPoints, 1.0f, glm::vec3(1.0f));

    for (const auto& planet : planets) {
        glm::vec2 pos(planet.position.x, planet.position.z);
        float scale = planet.radius * 2.0f; // visual scale

        glUniform1i(minimapShader.getUniformLocation("isPlanet"), 1);
        glUniform2fv(minimapShader.getUniformLocation("planetPos"), 1, glm::value_ptr(pos));
        glUniform3fv(minimapShader.getUniformLocation("planetColor"), 1, glm::value_ptr(planet.color));
        glUniform1f(minimapShader.getUniformLocation("planetScale"), scale);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    for (const auto &point : points) {
        glm::vec2 pos(point.position.x, point.position.z);
        float scale = point.radius; // visual scale

        glUniform1i(minimapShader.getUniformLocation("isPlanet"), 1);
        glUniform2fv(minimapShader.getUniformLocation("planetPos"), 1, glm::value_ptr(pos));
        glUniform3fv(minimapShader.getUniformLocation("planetColor"), 1, glm::value_ptr(point.color));
        glUniform1f(minimapShader.getUniformLocation("planetScale"), scale);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glm::vec2 pos(positionBattleCruiser.x, positionBattleCruiser.z);
    float scale = 5.0f; // visual scale
    glUniform1i(minimapShader.getUniformLocation("isPlanet"), 0);
    glUniform2fv(minimapShader.getUniformLocation("planetPos"), 1, glm::value_ptr(pos));
    glUniform3fv(minimapShader.getUniformLocation("planetColor"), 1, glm::value_ptr(glm::vec3(0.7f)));
    glUniform1f(minimapShader.getUniformLocation("planetScale"), scale);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Minimap::drawToScreen(int windowWidth, int windowHeight) {
    glDisable(GL_DEPTH_TEST);

    int mapSize = 512;
    int margin = 10;
    glViewport(windowWidth - mapSize - margin, margin, mapSize, mapSize);

    minimapScreenShader.bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, minimapColorTexture);
    glUniform1i(minimapScreenShader.getUniformLocation("minimapTex"), 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Restore viewport to full window
    glViewport(0, 0, windowWidth, windowHeight);
}

