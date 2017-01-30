#pragma once

#include "Command.h"

class NavigateCommand : public Command {
public:
	NavigateCommand();
	~NavigateCommand();

	void execute(VrInputState& currentInputState, VrInputState& lastInputState, Matrix4& worldTrans);

private:
	Vector3 initialLeftHandPos;
	Vector3 initialRightHandPos;
	Matrix4 initialWorldTrans;
	Matrix4 initialWorldInverse;

};

