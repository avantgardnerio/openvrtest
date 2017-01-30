#include "NavigateCommand.h"

#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>

#include "VrInput.h"

using namespace std;

NavigateCommand::NavigateCommand() {
}

NavigateCommand::~NavigateCommand() {
}

void NavigateCommand::execute(VrInputState& vrInputState, VrInputState& lastInputState, Matrix4& worldTrans) {
	if (
		(!(lastInputState.leftControllerState.ulButtonPressed & BTN_GRIP) || !(lastInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		&& ((vrInputState.leftControllerState.ulButtonPressed & BTN_GRIP) && (vrInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		) {
		initialWorldTrans = worldTrans;
		initialWorldInverse = initialWorldTrans;
		initialWorldInverse.invert();
		initialLeftHandPos = vrInputState.leftHandPose * Vector3(0, 0, 0);
		initialRightHandPos = vrInputState.rightHandPose * Vector3(0, 0, 0);
		initialLeftHandPos = initialWorldInverse * initialLeftHandPos;
		initialRightHandPos = initialWorldInverse * initialRightHandPos;
	}
	if (
		((lastInputState.leftControllerState.ulButtonPressed & BTN_GRIP) && (lastInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		&& ((vrInputState.leftControllerState.ulButtonPressed & BTN_GRIP) && (vrInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		) {
		Vector3 gripCenter = (initialRightHandPos - initialLeftHandPos) / 2.0f + initialLeftHandPos;
		Vector3 gripDir = initialRightHandPos - gripCenter;

		Vector3 leftHand = vrInputState.leftHandPose * Vector3(0, 0, 0);
		Vector3 rightHand = vrInputState.rightHandPose * Vector3(0, 0, 0);

		Vector3 worldLeft = initialWorldInverse * leftHand;
		Vector3 worldRight = initialWorldInverse * rightHand;
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

		worldTrans = initialWorldTrans * trans;
	}
}
