#include <iostream>
#include <vector>
#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <GL/glew.h>
#include <signal.h>
#include <algorithm>

#include "geom/Matrices.h"

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "GlContext.h"
#include "Controller.h"

using namespace std;
using namespace vr;

bool running = true;

void sig_handler(int signum) {
    running = false;
    printf("Received signal %d\n", signum);
}

int main() {
    signal(SIGINT, sig_handler);

    // Setup SDL
	SdlContext sdl;
	if (!sdl.init()) {
		return -1;
	}

	// Init GL
	GlContext gl;
	if (!gl.init()) {
		return -1;
	}

	// Create letterboxed monitor window
	SdlTargetWindow monitorWindow(571, 108, 777, 864);
	if (!monitorWindow.init()) {
		return -1;
	}

	// Init HMD
	VrInput vr;
	if (!vr.init()) {
		return -1;
	}

	// Init GL
	Controller leftController;
	if (!leftController.init()) {
		return -1;
	}
	Controller rightController;
	if (!rightController.init()) {
		return -1;
	}

    // Main loop
	VrInputState vrInputState;
    SDL_StartTextInput();
    while (running) {
        // SDL input
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent) != 0) {
            if (sdlEvent.type == SDL_QUIT) {
                running = false;
            }
            if (sdlEvent.type == SDL_KEYDOWN) {
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE || sdlEvent.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

		// Track devices
		vr.getState(vrInputState);
		leftController.setPose(vrInputState.leftHandPose);
		rightController.setPose(vrInputState.rightHandPose);

		// Render each perspective
		vr.render(leftController, vrInputState.headInverse);

        // Render to monitor window
		monitorWindow.render(vr.getLeftResolveId());

        // Submit to HMD
		vr.submitFrame();

        // Swap
		monitorWindow.swap();
    }

    // Cleanup
    cout << "exit!\n";

    return 0;
}

