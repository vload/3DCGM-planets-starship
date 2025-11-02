#include "Battlecruiser.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <random>
#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include "BezierPath.h"

BezierPath::BezierPath(Battlecruiser& battlecruiser)
    : battlecruiser(battlecruiser)
{
	//Create Quad covering the entire screen
	unsigned int quad_indices[6] = { 0, 1, 2, 3, 0, 2 };

	glm::vec3 quad_vertices[4] = {
		{-1.0f, -1.0f, 0.0f}, // bottom-left
		{ 1.0f, -1.0f, 0.0f}, // bottom-right
		{ 1.0f,  1.0f, 0.0f}, // top-right
		{-1.0f,  1.0f, 0.0f}  // top-left
	};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);

	bezierShader =
		ShaderBuilder()
		.addStage(GL_VERTEX_SHADER, RESOURCE_ROOT
			"shaders/battlecruiser/bezier_vertex.glsl")
		.addStage(GL_FRAGMENT_SHADER, RESOURCE_ROOT
			"shaders/battlecruiser/bezier_frag.glsl")
		.build();
}

BezierPath::~BezierPath()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
}

void BezierPath::draw(const glm::mat4& view, const glm::mat4& projection)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	bezierShader.bind();

	std::vector<glm::vec3> controlPoints;
	std::vector<BezierCurve> bezierCurves = battlecruiser.getBezierCurves();

	controlPoints.reserve(bezierCurves.size() * 4);
	for (const auto& c : bezierCurves)
	{
		controlPoints.push_back(c.p0);
		controlPoints.push_back(c.p1);
		controlPoints.push_back(c.p2);
		controlPoints.push_back(c.p3);
	}

	glUniform1i(bezierShader.getUniformLocation("numCurves"), (int)bezierCurves.size());
	glUniform3fv(bezierShader.getUniformLocation("curves"), (GLsizei)controlPoints.size(), glm::value_ptr(controlPoints[0]));
	glUniformMatrix4fv(bezierShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(battlecruiser.getModelMatrix()));
	glUniformMatrix4fv(bezierShader.getUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(bezierShader.getUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}