#include <signal.h>
#include <fstream>

#include <GL/glew.h>

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "Controller.h"
#include "Square.h"
#include "AssImpObj.h"
#include "NavigateCommand.h"

using namespace std;

enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};

int main() {
    // Setup
	NavigateCommand navigateCmd;
	SdlContext sdl;
	SdlTargetWindow monitorWindow(571, 108, 777, 864); // TODO: fix hard coded values for 1080 monitor
	VrInput vr;
	Controller leftController(Vector3(1, 0, 0));
	Controller rightController(Vector3(0, 0, 1));
	Square square;
	AssImpObj duck("..\\..\\..\\..\\assets\\duck.dae");

	// Init
	sdl.init();
	monitorWindow.init();
	vr.init();
	leftController.init();
	rightController.init();
	square.init();
	duck.init();

	// Build scene
	vector<Renderable*> scene;
	scene.push_back(&rightController);
	scene.push_back(&leftController);
	scene.push_back(&square);
	scene.push_back(&duck);

	// Initial state
	Matrix4 worldTrans;
	VrInputState vrInputState;
	VrInputState lastInputState;
	vrInputState.leftControllerState.ulButtonPressed = 0L;
	vrInputState.rightControllerState.ulButtonPressed = 0L;
	lastInputState.leftControllerState.ulButtonPressed = 0L;
	lastInputState.rightControllerState.ulButtonPressed = 0L;

	// Main loop
	SDL_StartTextInput();
    while (true) {
        // SDL input
		const Uint8* state = sdl.getState();
		if (state == NULL || state[SDLK_ESCAPE] != 0 || state[SDLK_q] != 0) {
			break;
		}

		// VR input
		vr.getState(vrInputState);
		leftController.setPose(vrInputState.leftHandPose);
		rightController.setPose(vrInputState.rightHandPose);

		// Navigation
		navigateCmd.execute(vrInputState, lastInputState, worldTrans);

		// Render each perspective to texture
		vr.render(scene, vrInputState.headInverse, worldTrans);

        // Render texture to monitor window & HMD
		monitorWindow.render(vr.getLeftResolveId());
		vr.submitFrame();
		monitorWindow.swap();
		lastInputState = vrInputState;
    }
}

