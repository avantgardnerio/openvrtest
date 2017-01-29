#pragma once

#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <stdio.h>

class SdlTargetWindow {
public:
	SdlTargetWindow(int left, int top, int width, int height);
	~SdlTargetWindow();

	bool init();
	void swap();

private:
	int left;
	int top;
	int width;
	int height;
	SDL_Window *monitorWindow;
	SDL_GLContext monitorGlContext;
};

