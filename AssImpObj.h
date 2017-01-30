#pragma once

#include <fstream>

#include <GL/glew.h>

#include "assimp/Importer.hpp"	
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

#include "Renderable.h"

using namespace std;
using namespace Assimp;

class AssImpObj : public Renderable {

public:
	AssImpObj(string filename);
	~AssImpObj();

	void init();
	
	void render(Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans);

private:
	string filename;
	Importer importer;
	const aiScene* model = NULL;

	void importModelFromFile(const string& pFile);
	void Color4f(const aiColor4D *color);
	void recursiveRender(const struct aiScene *sc, const struct aiNode* nd, Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans);
};

