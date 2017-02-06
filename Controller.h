#pragma once

#include <vector>

#include <GL/glew.h>

#include "geom/Matrices.h"
#include "GlContext.h"
#include "Renderable.h"

class Controller : public Renderable
{
public:
	Controller(Vector3 color);
	~Controller();

	void init();
	void render(Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans);
	void setPose(Matrix4 pose);

private:
	Matrix4 pose;
	Vector3 color;

	// TODO: static
	GLuint shader;
	GLint shaderMatrix;
	GLuint vertexArray;
	GLuint vertexBuffer;
	unsigned int vertexCount;
};

