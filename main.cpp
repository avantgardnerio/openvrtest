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

    // Setup
	SdlContext sdl;
	SdlTargetWindow monitorWindow(571, 108, 777, 864); // TODO: fix hard coded values for 1080 monitor
	VrInput vr;
	Controller leftController;
	Controller rightController;

	// Init
	sdl.init();
	monitorWindow.init();
	vr.init();
	leftController.init();
	rightController.init();

	// Build scene
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

