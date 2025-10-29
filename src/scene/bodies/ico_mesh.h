#pragma once

#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
DISABLE_WARNINGS_POP()
#include <framework/mesh.h>

#include <cmath>
#include <iostream>
#include <vector>

// Helper: create a CPU-side Mesh that represents a class I geodesic icosahedron
// centered at origin, with given frequency (>=1).
static Mesh generate_ico_mesh(int frequency) {
    Mesh mesh;
    if (frequency < 1) frequency = 1;

    const float t = (1.0f + sqrtf(5.0f)) * 0.5f;

    // Base icosahedron vertices
    std::vector<glm::vec3> baseVerts = {{-1, t, 0},  {1, t, 0},   {-1, -t, 0},
                                        {1, -t, 0},  {0, -1, t},  {0, 1, t},
                                        {0, -1, -t}, {0, 1, -t},  {t, 0, -1},
                                        {t, 0, 1},   {-t, 0, -1}, {-t, 0, 1}};
    for (auto& v : baseVerts) v = glm::normalize(v);

    // Base icosahedron faces
    const std::vector<glm::uvec3> baseFaces = {
        {0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
        {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
        {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1}};

    // const std::vector<glm::uvec3> baseFaces = {{0, 11, 5}};

    // Helper: interpolate between two vertices
    auto interpolate = [&](const glm::vec3& a, const glm::vec3& b, int i,
                           int n) {
        return glm::normalize(glm::mix(a, b, float(i) / float(n)));
    };

    std::vector<glm::vec3> positions;
    std::vector<glm::uvec3> tris;

    // Subdivide each face
    for (const auto& face : baseFaces) {
        glm::vec3 v0 = baseVerts[face.x];
        glm::vec3 v1 = baseVerts[face.y];
        glm::vec3 v2 = baseVerts[face.z];

        // Generate subdivided points on the face
        std::vector<std::vector<int>> grid(frequency + 1);

        for (int i = 0; i <= frequency; ++i) {
            glm::vec3 a = interpolate(v2, v0, i, frequency);
            glm::vec3 b = interpolate(v2, v1, i, frequency);

            for (int j = 0; j <= i; ++j) {
                glm::vec3 p = interpolate(a, b, j, i == 0 ? 1 : i);
                positions.push_back(p);
                grid[i].push_back((int)positions.size() - 1);
            }
        }

        // Create small triangles from the grid of points
        for (int i = 0; i < frequency; ++i) {
            for (int j = 0; j <= i; ++j) {
                int a = grid[i][j];
                int b = grid[i + 1][j];
                int c = grid[i + 1][j + 1];
                tris.push_back({a, b, c});

                if (j > 0) {
                    int d = grid[i][j - 1];
                    tris.push_back({a, d, b});
                }
            }
        }
    }

    // Deduplicate vertices
    std::vector<glm::vec3> uniqueVerts;
    std::vector<int> remap(positions.size(), -1);
    float eps = 1e-5f;
    for (size_t i = 0; i < positions.size(); ++i) {
        const glm::vec3& p = positions[i];
        bool found = false;
        for (size_t j = 0; j < uniqueVerts.size(); ++j) {
            if (glm::length(uniqueVerts[j] - p) < eps) {
                remap[i] = (int)j;
                found = true;
                break;
            }
        }
        if (!found) {
            remap[i] = (int)uniqueVerts.size();
            uniqueVerts.push_back(p);
        }
    }
    for (auto& tri : tris) {
        tri.x = remap[tri.x];
        tri.y = remap[tri.y];
        tri.z = remap[tri.z];
    }

    // Build vertex list
    mesh.vertices.reserve(uniqueVerts.size());
    for (const auto& p : uniqueVerts) {
        Vertex v;
        v.position = glm::normalize(p);
        v.normal = glm::normalize(p);
        v.texCoord = glm::vec2(0.0f);
        mesh.vertices.push_back(v);
    }

    mesh.triangles = tris;
    mesh.material.kd = glm::vec3(0.8f, 0.6f, 0.3f);
    return mesh;
}