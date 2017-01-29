#include "VrInput.h"

VrInput::VrInput() {
	error = VRInitError_None;
}

VrInput::~VrInput() {
	VR_Shutdown();
}

bool VrInput::init() {
	hmd = VR_Init(&error, VRApplication_Scene);
	if (error != VRInitError_None) {
		printf("Unable to init VR runtime: %s", VR_GetVRInitErrorAsEnglishDescription(error));
		return false;
	}
	hmd->GetRecommendedRenderTargetSize(&width, &height);
	if (!VRCompositor()) {
		printf("Compositor initialization failed. See log file for details\n", __FUNCTION__);
		return false;
	}
	eyeProjLeft = getEyeMat(Eye_Left);
	eyeProjRight = getEyeMat(Eye_Right);

	GlContext::createFrameBuffer(width, height, eyeFramebuffer[Eye_Left]);
	GlContext::createFrameBuffer(width, height, eyeFramebuffer[Eye_Right]);

	return true;
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
		if (role == TrackedControllerRole_RightHand) {
			state.rightHandPose = poseMat;
		}
		if (role == TrackedControllerRole_LeftHand) {
			state.leftHandPose = poseMat;
		}
		if (clazz == TrackedDeviceClass_HMD) {
			state.headPose = poseMat;
			state.headInverse = poseMat;
			state.headInverse.invert();
		}
	}
}

void VrInput::render(Renderable& renderable, Matrix4 proj) {
	renderPerspective(Eye_Left, renderable, getEyeProjLeft() * proj);
	renderPerspective(Eye_Right, renderable, getEyeProjRight() * proj);
}

void VrInput::renderPerspective(EVREye eye, Renderable& renderable, Matrix4 proj) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_MULTISAMPLE);
	glBindFramebuffer(GL_FRAMEBUFFER, eyeFramebuffer[eye].renderTextureId);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	renderable.render(proj);

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
