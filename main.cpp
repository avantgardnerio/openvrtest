#include <signal.h>

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "Controller.h"

using namespace std;

int main() {
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
    while (true) {
        // SDL input
		const Uint8* state = sdl.getState();
		if (state == NULL) {
			break;
		}
		if (state[SDLK_ESCAPE] || state[SDLK_q]) {
			break;
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
}

