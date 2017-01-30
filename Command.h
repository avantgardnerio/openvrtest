#pragma once

#include "geom/Matrices.h"

#include "VrInput.h"

class Command {
public:
	virtual void execute(VrInputState& currentInputState, VrInputState& lastInputState, Matrix4& worldTrans) = 0;
};