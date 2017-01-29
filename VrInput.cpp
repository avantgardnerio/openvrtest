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

	GlContext::createFrameBuffer(width, height, leftEyeDesc);
	GlContext::createFrameBuffer(width, height, rightEyeDesc);

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

void VrInput::submitFrame() {
	Texture_t leftEyeTexture = { (void *)leftEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma };
	VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
	Texture_t rightEyeTexture = { (void *)rightEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma };
	VRCompositor()->Submit(Eye_Right, &rightEyeTexture);
}

GLuint VrInput::getLeftRenderId() {
	return leftEyeDesc.renderFramebufferId;
}

GLuint VrInput::getRightRenderId() {
	return rightEyeDesc.renderFramebufferId;
}

GLuint VrInput::getLeftResolveId() {
	return leftEyeDesc.resolveFramebufferId;
}

GLuint VrInput::getRightResolveId() {
	return rightEyeDesc.resolveFramebufferId;
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
