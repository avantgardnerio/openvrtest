#pragma once

#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <stdio.h>
#include <algorithm>

class SdlContext
{
public:
	SdlContext();
	~SdlContext();

	bool init();

	int getWidth();
	int getHeight();

private:
	int width;
	int height;
};

