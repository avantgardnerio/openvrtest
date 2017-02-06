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
		initialLeftHandPos = vrInputState.leftHandPose * Vector4(0, 0, 0, 1);
		initialRightHandPos = vrInputState.rightHandPose * Vector4(0, 0, 0, 1);
		initialLeftHandPos = initialWorldInverse * initialLeftHandPos;
		initialRightHandPos = initialWorldInverse * initialRightHandPos;
	}
	if (
		((lastInputState.leftControllerState.ulButtonPressed & BTN_GRIP) && (lastInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		&& ((vrInputState.leftControllerState.ulButtonPressed & BTN_GRIP) && (vrInputState.rightControllerState.ulButtonPressed & BTN_GRIP))
		) {
		Vector4 gripCenter = (initialRightHandPos - initialLeftHandPos) / 2.0f + initialLeftHandPos;
		Vector4 gripDir = initialRightHandPos - gripCenter;

		Vector4 leftHand = vrInputState.leftHandPose * Vector4(0, 0, 0, 1);
		Vector4 rightHand = vrInputState.rightHandPose * Vector4(0, 0, 0, 1);

		Vector4 worldLeft = initialWorldInverse * leftHand;
		Vector4 worldRight = initialWorldInverse * rightHand;
		Vector4 worldCenter = (worldRight - worldLeft) / 2.0f + worldLeft;
		Vector4 worldDir = worldRight - worldCenter;
		Vector4 delta = worldCenter - gripCenter;
		float deltaAng = atan2f(gripDir.z, gripDir.x) - atan2f(worldDir.z, worldDir.x);
		float deltaDist = worldDir.length() / gripDir.length();

		Vector4 origin = initialWorldTrans * Vector4(0,0,0, 1);
		Vector4 unit = initialWorldTrans * Vector4(1,0,0, 1);
		float scale = (unit - origin).length();

		Matrix4 trans;

		trans.translateRaw(-gripCenter);
		trans.rotateY(deltaAng * 180.0f / (float)M_PI);
		trans.scale(deltaDist);
		trans.translateRaw(gripCenter);

		trans.translateRaw(delta);
		//cout << "delta=" << delta << "\r\n";

		worldTrans = initialWorldTrans * trans;
	}
}
