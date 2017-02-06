#include <signal.h>
#include <fstream>

#include <GL/glew.h>

#include <nanogui/formhelper.h>
#include <nanogui/nanogui.h>

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "Controller.h"
#include "Square.h"
#include "AssImpObj.h"
#include "NavigateCommand.h"

using namespace std;
using namespace nanogui;

int main() {
	nanogui::init();
	
	FormHelper *gui;
		/*= new FormHelper(screen);
	ref<Window> window = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
	gui->addGroup("Basic types");
	gui->addVariable("bool", bvar);
	gui->addVariable("string", strvar);

	gui->addGroup("Validating fields");
	gui->addVariable("int", ivar);
	gui->addVariable("float", fvar);
	gui->addVariable("double", dvar);

	gui->addGroup("Complex types");
	gui->addVariable("Enumeration", enumval, enabled)
	->setItems({"Item 1", "Item 2", "Item 3"});
	gui->addVariable("Color", colval);

	gui->addGroup("Other widgets");
	gui->addButton("A button", [](){ std::cout << "Button pressed." << std::endl; });
	
	screen->setVisible(true);
	screen->performLayout();
	window->center();
	*/
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

