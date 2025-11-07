// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
#include <framework/shader.h>
#include <scene/bodies/Body.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

struct Object {
    glm::vec3 position;
    float radius;
    glm::vec3 color;
};

class Minimap {

public:
    Minimap();
    ~Minimap();

    void initBuffers();
    void draw(const std::vector<Body*>& bodies, const std::vector<glm::vec3>& positionPoints, glm::vec3 positionBattleCruiser);
    void drawToScreen(int windowWidth, int windowHeight);
    virtual void imGuiControl();

private:
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint minimapFrameBuffer;
    GLuint minimapColorTexture;
    GLuint minimapDepthBuffer;

    Shader minimapShader;
    Shader minimapScreenShader;

    bool displayMinimap = true;

    static std::vector<Object> convertBodyToObjects(const std::vector<Body*>& bodies) {
        std::vector<Object> objects;
        objects.reserve(bodies.size());
        for (const auto* body : bodies) {
            if (!body) continue;
            objects.push_back({
                body->getPosition(),
                body->getRadius(),
                body->getColor()
            });
        }
        return objects;
    }

    static std::vector<Object> convertBezierPointsToObjects(const std::vector<glm::vec3>& positionPoints, float radius, glm::vec3 color) {
        std::vector<Object> objects;
        objects.reserve(positionPoints.size());

        for (const auto& pos : positionPoints) {
            objects.push_back({
                pos,
                radius,
                color
            });
        }

        return objects;
    }
};
