#include "VrInput.h"

#include <fstream>

#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/LogStream.hpp"

VrInput::VrInput() {
	error = VRInitError_None;
	BTN_GRIP = ButtonMaskFromId(k_EButton_Grip);
}

VrInput::~VrInput() {
	VR_Shutdown();
}

Assimp::Importer importer;
const aiScene* theScene = NULL;

void importModelFromFile(const std::string& pFile) {
	// Check if file exists
	std::ifstream fin(pFile.c_str());
	if (!fin.fail()) {
		fin.close();
	}
	else {
		throw exception("Error opening file");
	}

	theScene = importer.ReadFile(pFile, aiProcessPreset_TargetRealtime_Quality);

	// If the import failed, report it
	if (!theScene) {
		throw exception(importer.GetErrorString());
	}
}

void Color4f(const aiColor4D *color) {
	glColor4f(color->r, color->g, color->b, color->a);
}

void recursive_render(const struct aiScene *sc, const struct aiNode* nd, float scale, Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	unsigned int i;
	unsigned int n = 0, t;
	aiMatrix4x4 m = nd->mTransformation;

	aiMatrix4x4 m2;
	aiMatrix4x4::Scaling(aiVector3D(scale, scale, scale), m2);
	m = m * m2;

	// update transform
	m.Transpose();
	glPushMatrix();
	glMultMatrixf((eyeProj * headInverse * worldTrans).get());

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = theScene->mMeshes[nd->mMeshes[n]];

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

		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch (face->mNumIndices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for (i = 0; i < face->mNumIndices; i++) {	// go through all vertices in face
				int vertexIndex = face->mIndices[i];	// get group index for current index
				if (mesh->mColors[0] != NULL)
					Color4f(&mesh->mColors[0][vertexIndex]);
				if (mesh->mNormals != NULL)

					if (mesh->HasTextureCoords(0))		//HasTextureCoords(texture_coordinates_set)
					{
						glTexCoord2f(mesh->mTextureCoords[0][vertexIndex].x, 1 - mesh->mTextureCoords[0][vertexIndex].y); //mTextureCoords[channel][vertex]
					}

				glNormal3fv(&mesh->mNormals[vertexIndex].x);
				glVertex3fv(&mesh->mVertices[vertexIndex].x);
			}
			glEnd();
		}
	}
	glPopMatrix();

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		recursive_render(sc, nd->mChildren[n], scale, eyeProj, headInverse, worldTrans);
	}

}

void drawAiScene(const aiScene* scene, Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	recursive_render(scene, scene->mRootNode, 4, eyeProj, headInverse, worldTrans);
}

void VrInput::init() {
	hmd = VR_Init(&error, VRApplication_Scene);
	if (error != VRInitError_None) {
		throw new exception(VR_GetVRInitErrorAsEnglishDescription(error));
	}
	hmd->GetRecommendedRenderTargetSize(&width, &height);
	if (!VRCompositor()) {
		throw new exception("Compositor initialization failed. See log file for details\n");
	}
	eyeProjLeft = getEyeMat(Eye_Left);
	eyeProjRight = getEyeMat(Eye_Right);

	GlContext::createFrameBuffer(width, height, eyeFramebuffer[Eye_Left]);
	GlContext::createFrameBuffer(width, height, eyeFramebuffer[Eye_Right]);

	string modelpath = std::string("..\\..\\..\\..\\assets\\duck.dae");
	importModelFromFile(modelpath);
}

void VrInput::getState(VrInputState& state) {
	VREvent_t event;
	while (hmd->PollNextEvent(&event, sizeof(event))) {
		if (event.eventType == VREvent_TrackedDeviceActivated) {
			//initDeviceModel(event.trackedDeviceIndex);
		}
	}

	// Update
	VRCompositor()->WaitGetPoses(devicePose, k_unMaxTrackedDeviceCount, NULL, 0);
	for (int deviceIdx = 0; deviceIdx < k_unMaxTrackedDeviceCount; ++deviceIdx) {
		const TrackedDevicePose_t &pose = devicePose[deviceIdx];
		if (!pose.bPoseIsValid) {
			continue;
		}
		Matrix4 poseMat = hmdMatToMatrix4(pose.mDeviceToAbsoluteTracking);
		ETrackedDeviceClass clazz = hmd->GetTrackedDeviceClass(deviceIdx);
		ETrackedControllerRole role = hmd->GetControllerRoleForTrackedDeviceIndex(deviceIdx);
		VRControllerState_t controllerState;
		hmd->GetControllerState(deviceIdx, &controllerState, sizeof(controllerState));
		if (role == TrackedControllerRole_RightHand) {
			state.rightHandPose = poseMat;
			state.leftControllerState = controllerState;
		}
		if (role == TrackedControllerRole_LeftHand) {
			state.leftHandPose = poseMat;
			state.rightControllerState = controllerState;
		}
		if (clazz == TrackedDeviceClass_HMD) {
			state.headPose = poseMat;
			state.headInverse = poseMat;
			state.headInverse.invert();
		}
	}
}

void VrInput::render(vector<Renderable*>& scene, Matrix4 headInverse, Matrix4 worldTrans) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	renderPerspective(Eye_Left, scene, getEyeProjLeft(), headInverse, worldTrans);
	renderPerspective(Eye_Right, scene, getEyeProjRight(), headInverse, worldTrans);
}

void VrInput::renderPerspective(EVREye eye, vector<Renderable*>& scene, Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	glEnable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_FRAMEBUFFER, eyeFramebuffer[eye].renderTextureId);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	for (Renderable* renderable : scene) {
		renderable->render(eyeProj, headInverse, worldTrans);
	}
	drawAiScene(theScene, eyeProj, headInverse, worldTrans);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, eyeFramebuffer[eye].renderTextureId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeFramebuffer[eye].resolveTextureId);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glEnable(GL_MULTISAMPLE);
}

void VrInput::submitFrame() {
	Texture_t leftEyeTexture = { (void *)eyeFramebuffer[Eye_Left].resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma };
	VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
	Texture_t rightEyeTexture = { (void *)eyeFramebuffer[Eye_Right].resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma };
	VRCompositor()->Submit(Eye_Right, &rightEyeTexture);
}

GLuint VrInput::getLeftRenderId() {
	return eyeFramebuffer[Eye_Left].renderFramebufferId;
}

GLuint VrInput::getRightRenderId() {
	return eyeFramebuffer[Eye_Right].renderFramebufferId;
}

GLuint VrInput::getLeftResolveId() {
	return eyeFramebuffer[Eye_Left].resolveFramebufferId;
}

GLuint VrInput::getRightResolveId() {
	return eyeFramebuffer[Eye_Right].resolveFramebufferId;
}

uint32_t VrInput::getWidth() {
	return width;
}

uint32_t VrInput::getHeight() {
	return height;
}

Matrix4 VrInput::getEyeProjLeft() {
	return eyeProjLeft;
}

Matrix4 VrInput::getEyeProjRight() {
	return eyeProjRight;
}

Matrix4 VrInput::getEyeMat(Hmd_Eye eye) {
	float nearClip = 0.1f;
	float farClip = 30.0f;
	HmdMatrix44_t projHmd = hmd->GetProjectionMatrix(eye, nearClip, farClip);
	Matrix4 proj = hmdMatToMatrix4(projHmd);
	Matrix4 trans = hmdMatToMatrix4(hmd->GetEyeToHeadTransform(eye)).invert();
	return proj * trans;
}

Matrix4 VrInput::hmdMatToMatrix4(const HmdMatrix34_t &mat) {
	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0,
		mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0,
		mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0,
		mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
	);
}

Matrix4 VrInput::hmdMatToMatrix4(const HmdMatrix44_t &mat) {
	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}
