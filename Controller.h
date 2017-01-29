#pragma once

#include <vector>

#include <GL/glew.h>

#include "geom/Matrices.h"
#include "GlContext.h"

class Controller
{
public:
	Controller();
	~Controller();

	bool init();
	void render(Matrix4 proj);
	void setPose(Matrix4 pose);

private:
	Matrix4 pose;

	// TODO: static
	GLuint controllerShader;
	GLint controllerShaderMatrix;
	GLuint controllerVertAr;
	GLuint controllerVertBuffer;
	unsigned int controllerVertCount;
};

