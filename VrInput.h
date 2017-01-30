#pragma once

#include <vector>

#include "openvr.h"

#include "geom/Matrices.h"
#include "GlContext.h"
#include "Renderable.h"

using namespace std;
using namespace vr;

struct VrInputState {
	Matrix4 headPose;
	Matrix4 headInverse;
	Matrix4 leftHandPose;
	Matrix4 rightHandPose;
};

class VrInput
{
public:
	VrInput();
	~VrInput();

	bool init();
	void getState(VrInputState& state);
	void render(vector<Renderable*>& renderable, Matrix4 proj);
	void renderPerspective(EVREye eye, vector<Renderable*>& renderable, Matrix4 proj);
	void submitFrame();

	uint32_t getWidth();
	uint32_t getHeight();

	GLuint getLeftRenderId();
	GLuint getRightRenderId();
	GLuint getLeftResolveId();
	GLuint getRightResolveId();

	Matrix4 getEyeProjLeft();
	Matrix4 getEyeProjRight();

private:
	TrackedDevicePose_t devicePose[k_unMaxTrackedDeviceCount];

	EVRInitError error;
	IVRSystem *hmd;

	uint32_t width;
	uint32_t height;
	FramebufferDesc eyeFramebuffer[2];

	Matrix4 eyeProjLeft;
	Matrix4 eyeProjRight;

	// Methods
	Matrix4 getEyeMat(Hmd_Eye eye);
	Matrix4 hmdMatToMatrix4(const HmdMatrix34_t &mat);
	Matrix4 hmdMatToMatrix4(const HmdMatrix44_t &mat);
};

