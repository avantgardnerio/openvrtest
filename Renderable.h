#pragma once

#include "geom/Matrices.h"

class Renderable {
public:
	virtual void render(Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) = 0;
};