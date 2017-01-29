#include "SdlTargetWindow.h"

SdlTargetWindow::SdlTargetWindow(int left, int top, int width, int height) {
	this->left = left;
	this->top = top;
	this->width = width;
	this->height = height;
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
	return true;
}

SdlTargetWindow::~SdlTargetWindow() {
}

void SdlTargetWindow::swap() {
	SDL_GL_SwapWindow(monitorWindow);
}

