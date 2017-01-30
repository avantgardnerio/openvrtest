#include "SdlContext.h"

using namespace std;

SdlContext::SdlContext() {
	width = INT_MAX;
	height = INT_MAX;
}

SdlContext::~SdlContext() {
	SDL_StopTextInput();
}

void SdlContext::init() {
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

	if (SDL_VideoInit(NULL) != 0) {
		throw new exception(SDL_GetError());
	}

	SDL_DisplayMode current;
	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
		int ret = SDL_GetCurrentDisplayMode(i, &current);
		if (ret != 0) {
			throw new exception(SDL_GetError());
		} else {
			width = min(current.w, width);
			height = min(current.h, height);
		}
	}
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
