#pragma once

#include <vector>
#include <GL/glew.h>

#include "Renderable.h"
#include "GlContext.h"

class Square :
	public Renderable
{
public:
	Square();
	~Square();

	void init();
	void render(Matrix4 proj);

private:
	GLuint shader;
	GLint shaderMatrix;
	GLuint vertexArray;
	GLuint vertexBuffer;
	unsigned int vertexCount;

};

