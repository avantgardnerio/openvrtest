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
	SdlTargetWindow monitorWindow(571, 108, 777, 864); // TODO: fix hard coded values for 1080 monitor
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
	vector<Renderable*> scene;
	scene.push_back(&rightController);
	scene.push_back(&leftController);

    // Main loop
	VrInputState vrInputState;
    SDL_StartTextInput();
    while (running) {
        // SDL input
		const Uint8* state = sdl.getState();
		if (state == NULL) {
			running = false;
			break;
		}
		if (state[SDLK_ESCAPE] || state[SDLK_q]) {
			running = false;
		}

		// VR input
		vr.getState(vrInputState);
		leftController.setPose(vrInputState.leftHandPose);
		rightController.setPose(vrInputState.rightHandPose);

		// Render each perspective to texture
		vr.render(scene, vrInputState.headInverse);

        // Render texture to monitor window & HMD
		monitorWindow.render(vr.getLeftResolveId());
		vr.submitFrame();
		monitorWindow.swap();
    }

    return 0;
}

