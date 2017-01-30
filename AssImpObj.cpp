#include "AssImpObj.h"

AssImpObj::AssImpObj(string filename) {
	this->filename = filename;
}

AssImpObj::~AssImpObj() {
}

void AssImpObj::init() {
	ifstream file(filename.c_str());
	if (!file.fail()) {
		file.close();
	} else {
		throw exception("Error opening file");
	}

	model = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);
	if (!model) {
		throw exception(importer.GetErrorString());
	}
}

void AssImpObj::recursiveRender(const struct aiScene *scene, const struct aiNode* node, Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	aiMatrix4x4 nodeMat = node->mTransformation;

	// update transform
	nodeMat.Transpose();
	glPushMatrix();
	glMultMatrixf((eyeProj * headInverse * worldTrans).get());

	// draw all meshes assigned to this node
	for (int meshIdx = 0; meshIdx < node->mNumMeshes; ++meshIdx) {
		const struct aiMesh* mesh = model->mMeshes[node->mMeshes[meshIdx]];

		//apply_material(sc->mMaterials[mesh->mMaterialIndex]); // TODO: Materials

		//if (mesh->mNormals == NULL) {
		glDisable(GL_LIGHTING);
		/*} else {
		glEnable(GL_LIGHTING);
		}*/

		//if (mesh->mColors[0] != NULL) {
		glEnable(GL_COLOR_MATERIAL);
		/*} else {
		glDisable(GL_COLOR_MATERIAL);
		}*/

		for (int faceIdx = 0; faceIdx < mesh->mNumFaces; ++faceIdx) {
			const struct aiFace* face = &mesh->mFaces[faceIdx];

			GLenum faceType;
			switch (face->mNumIndices) {
				case 1: faceType = GL_POINTS; break;
				case 2: faceType = GL_LINES; break;
				case 3: faceType = GL_TRIANGLES; break;
				default: faceType = GL_POLYGON; break;
			}

			glBegin(faceType);
			for (int vertIdx = 0; vertIdx < face->mNumIndices; vertIdx++) {	// go through all vertices in face
				int vertexIndex = face->mIndices[vertIdx];	// get group index for current index
				if (mesh->mColors[0] != NULL) {
					Color4f(&mesh->mColors[0][vertexIndex]);
				}
				if (mesh->mNormals != NULL && mesh->HasTextureCoords(0) == true) { //HasTextureCoords(texture_coordinates_set)
					glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
				}
				glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}
	glPopMatrix(); // Should go below children

	// draw all children
	for (int childIdx = 0; childIdx < node->mNumChildren; ++childIdx) {
		recursiveRender(scene, node->mChildren[childIdx], eyeProj, headInverse, worldTrans);
	}

}

void AssImpObj::Color4f(const aiColor4D *color) {
	glColor4f(color->r, color->g, color->b, color->a);
}

void AssImpObj::render(Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	recursiveRender(model, model->mRootNode, eyeProj, headInverse, worldTrans);
}

