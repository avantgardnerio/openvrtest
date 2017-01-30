#pragma once

#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <stdio.h>
#include <GL/glew.h>
#include <vector>

#include "geom/Matrices.h"
#include "GlContext.h"

class SdlTargetWindow {
public:
	SdlTargetWindow(int left, int top, int width, int height);
	~SdlTargetWindow();

	void init();
	void swap();
	void render(GLuint textureId);

private:
	int left;
	int top;
	int width;
	int height;
	SDL_Window *monitorWindow;
	SDL_GLContext monitorGlContext;
	GLuint windowShader;
	GLuint windowQuadVertAr;
	GLuint monitorWinVertBuff;
	GLuint monitorWinIdxBuff;
	unsigned int windowQuadIdxSize;

};

