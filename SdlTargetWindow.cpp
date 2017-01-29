#include "SdlTargetWindow.h"

using namespace std;

SdlTargetWindow::SdlTargetWindow(int left, int top, int width, int height) {
	this->left = left;
	this->top = top;
	this->width = width;
	this->height = height;

	windowQuadVertAr = 0;
	monitorWinVertBuff = 0;
	monitorWinIdxBuff = 0;
}

bool SdlTargetWindow::init() {
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	monitorWindow = SDL_CreateWindow("hellovr", left, top, width, height, unWindowFlags);
	if (monitorWindow == NULL) {
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}
	monitorGlContext = SDL_GL_CreateContext(monitorWindow);
	if (monitorGlContext == NULL) {
		printf("%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// HACK: Glew needs to be inited after creating the window and before compiling the shader. Find a better place for this.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(glewError));
		return false;
	}
	glGetError();

	windowShader = GlContext::compileGlShader(
		"MonitorWindow",

		"#version 410 core\n"
		"\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main() {\n"
		"\tv2UV = v2UVIn;\n"
		"\tgl_Position = position;\n"
		"}",

		"#version 410 core\n"
		"\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main() {\n"
		"\t\toutputColor = texture(mytexture, v2UV);\n"
		"}"
	);

	vector<TexturedVertex> verts;
	verts.push_back(TexturedVertex(Vector2(-1, -1), Vector2(0, 1)));
	verts.push_back(TexturedVertex(Vector2(1, -1), Vector2(1, 1)));
	verts.push_back(TexturedVertex(Vector2(-1, 1), Vector2(0, 0)));
	verts.push_back(TexturedVertex(Vector2(1, 1), Vector2(1, 0)));

	GLushort indices[] = { 0, 1, 3, 0, 3, 2 };
	windowQuadIdxSize = _countof(indices);

	glGenVertexArrays(1, &windowQuadVertAr);
	glBindVertexArray(windowQuadVertAr);

	glGenBuffers(1, &monitorWinVertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, monitorWinVertBuff);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(TexturedVertex), &verts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &monitorWinIdxBuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, monitorWinIdxBuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, windowQuadIdxSize * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)offsetof(TexturedVertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)offsetof(TexturedVertex, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

SdlTargetWindow::~SdlTargetWindow() {
}

void SdlTargetWindow::render(GLuint textureId) {
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glBindVertexArray(windowQuadVertAr);
	glUseProgram(windowShader);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, windowQuadIdxSize, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void SdlTargetWindow::swap() {
	SDL_GL_SwapWindow(monitorWindow);
}

