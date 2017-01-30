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
		int ret = SDL_GetCurrentDisplayMode(i, &current);
		if (ret != 0) {
			printf("Could not get display mode for video display #%d: %s", i, SDL_GetError()); // TODO: Error handling
		} else {
			width = min(current.w, width);
			height = min(current.h, height);
			printf("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h,	current.refresh_rate); // TODO: Error handling
		}
	}

	return true;
}

const Uint8 *SdlContext::getState() {
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent) != 0) {
		if (sdlEvent.type == SDL_QUIT) {
			return NULL; // TODO: Event dispatching
		}
	}
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	return state; // TODO: Translate state into native structure?
}


int SdlContext::getWidth() {
	return width; // TODO: Size structure
}

int SdlContext::getHeight() {
	return height; // TODO: Size structure
}
