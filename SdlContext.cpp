#include "SdlContext.h"

using namespace std;

SdlContext::SdlContext() {
	width = INT_MAX;
	height = INT_MAX;
}

SdlContext::~SdlContext() {
	SDL_StopTextInput();
}

bool SdlContext::init() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	if (SDL_VideoInit(NULL) != 0) {
		printf("Error initializing SDL video:  %s\n", SDL_GetError());
		return false;
	}

	SDL_DisplayMode current;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
		int should_be_zero = SDL_GetCurrentDisplayMode(i, &current);
		if (should_be_zero != 0) {
			printf("Could not get display mode for video display #%d: %s", i, SDL_GetError());
		}
		else {
			width = min(current.w, width);
			height = min(current.h, height);
			printf("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h,
				current.refresh_rate);
		}
	}

	return true;
}

int SdlContext::getWidth() {
	return width;
}

int SdlContext::getHeight() {
	return height;
}
