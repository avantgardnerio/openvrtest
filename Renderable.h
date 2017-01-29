#pragma once

#include "geom/Matrices.h"

class Renderable {
public:
	virtual void render(Matrix4 proj) = 0;
};