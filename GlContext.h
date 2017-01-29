#pragma once

#include <GL/glew.h>
#include <stdio.h>

#include "geom/Matrices.h"

struct TexturedVertex {
	Vector2 position;
	Vector2 texCoord;

	TexturedVertex(const Vector2 &pos, const Vector2 tex) : position(pos), texCoord(tex) {}
};

struct FramebufferDesc {
	GLuint depthBufferId;
	GLuint renderTextureId;
	GLuint renderFramebufferId;
	GLuint resolveTextureId;
	GLuint resolveFramebufferId;
};

class GlContext
{
public:
	GlContext();
	~GlContext();

	bool init();

	static GLuint GlContext::compileGlShader(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader);
	static bool GlContext::createFrameBuffer(int width, int height, FramebufferDesc &framebufferDesc);
};

