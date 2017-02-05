#pragma once

#include "Command.h"

class NavigateCommand : public Command {
public:
	NavigateCommand();
	~NavigateCommand();

	void execute(VrInputState& currentInputState, VrInputState& lastInputState, Matrix4& worldTrans);

private:
	Vector4 initialLeftHandPos;
	Vector4 initialRightHandPos;
	Matrix4 initialWorldTrans;
	Matrix4 initialWorldInverse;

};

