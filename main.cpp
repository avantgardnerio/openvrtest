#include <signal.h>
#include <fstream>

#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "Controller.h"
#include "Square.h"

using namespace std;

Assimp::Importer importer;
const aiScene* scene = NULL;

bool importModelFromFile(const std::string& pFile) {
	// Check if file exists
	std::ifstream fin(pFile.c_str());
	if (!fin.fail()) {
		fin.close();
	} else {
		printf("Error opening file");
		return -1;
	}

	scene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality);

	// If the import failed, report it
	if (!scene)	{
		printf(importer.GetErrorString());
		return false;
	}

	return true;
}

int main() {
    // Setup
	SdlContext sdl;
	SdlTargetWindow monitorWindow(571, 108, 777, 864); // TODO: fix hard coded values for 1080 monitor
	VrInput vr;
	Controller leftController;
	Controller rightController;
	Square square;

	// Init
	sdl.init();
	monitorWindow.init();
	vr.init();
	leftController.init();
	rightController.init();
	square.init();

	string modelpath = std::string("../../../../../assets/duck.dae");
	if (!importModelFromFile(modelpath)) return 0;

	// Build scene
	vector<Renderable*> scene;
	scene.push_back(&rightController);
	scene.push_back(&leftController);
	scene.push_back(&square);

    // Main loop
	Matrix4 worldTrans;
	Vector3 gripLeft;
	Vector3 gripRight;
	Matrix4 gripWorld;
	Matrix4 gripInverse;
	VrInputState vrInputState;
	VrInputState lastInputState;
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
		if (
			(!(lastInputState.leftControllerState.ulButtonPressed & vr.BTN_GRIP) || !(lastInputState.rightControllerState.ulButtonPressed & vr.BTN_GRIP))
			&& ((vrInputState.leftControllerState.ulButtonPressed & vr.BTN_GRIP) && (vrInputState.rightControllerState.ulButtonPressed & vr.BTN_GRIP))
			) {
			gripWorld = worldTrans;
			gripInverse = gripWorld;
			gripInverse.invert();
			gripLeft = vrInputState.leftHandPose * Vector3(0, 0, 0);
			gripRight = vrInputState.rightHandPose * Vector3(0, 0, 0);
			gripLeft = gripInverse * gripLeft;
			gripRight = gripInverse * gripRight;
		}
		if (
			((lastInputState.leftControllerState.ulButtonPressed & vr.BTN_GRIP) && (lastInputState.rightControllerState.ulButtonPressed & vr.BTN_GRIP))
			&& ((vrInputState.leftControllerState.ulButtonPressed & vr.BTN_GRIP) && (vrInputState.rightControllerState.ulButtonPressed & vr.BTN_GRIP))
			) {
			Vector3 gripCenter = (gripRight - gripLeft) / 2.0f + gripLeft;
			Vector3 gripDir = gripRight - gripCenter;

			Vector3 leftHand = vrInputState.leftHandPose * Vector3(0, 0, 0);
			Vector3 rightHand = vrInputState.rightHandPose * Vector3(0, 0, 0);

			Vector3 worldLeft = gripInverse * leftHand;
			Vector3 worldRight = gripInverse * rightHand;
			Vector3 worldCenter = (worldRight - worldLeft) / 2.0f + worldLeft;
			Vector3 worldDir = worldRight - worldCenter;
			Vector3 delta = worldCenter - gripCenter;
			float deltaAng = atan2f(gripDir.z, gripDir.x) - atan2f(worldDir.z, worldDir.x);
			float deltaDist = worldDir.length() / gripDir.length();

			Matrix4 trans;

			trans.translate(-gripCenter);
			trans.rotateY(deltaAng * 180.0f / M_PI);
			trans.scale(deltaDist);
			trans.translate(gripCenter);

			trans.translate(delta);

			worldTrans = gripWorld * trans;
		}

		// Render each perspective to texture
		vr.render(scene, vrInputState.headInverse, worldTrans);

        // Render texture to monitor window & HMD
		monitorWindow.render(vr.getLeftResolveId());
		vr.submitFrame();
		monitorWindow.swap();
		lastInputState = vrInputState;
    }
}

