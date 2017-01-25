#include <iostream>

#include "openvr.h"

#include "geom/Matrices.h"

Matrix4 steamMatToMatrix4( const vr::HmdMatrix34_t &matPose ) {
    Matrix4 matrixObj(
            matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
            matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
            matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
            matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
    );
    return matrixObj;
}

bool running = true;

void sig_handler(int signum) {
    running = false;
    printf("Received signal %d\n", signum);
}

int main() {
    signal(SIGINT, sig_handler);

    // Init HMD
    vr::EVRInitError eError = vr::VRInitError_None;
    vr::IVRSystem *hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None) {
        printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        return -1;
    }
    if (!vr::VRCompositor()) {
        printf("Compositor initialization failed. See log file for details\n", __FUNCTION__);
        return -1;
    }

    // Main loop
    vr::TrackedDevicePose_t devicePose[ vr::k_unMaxTrackedDeviceCount ];
    Matrix4 devicePoseMat[ vr::k_unMaxTrackedDeviceCount ];
    Matrix4 leftHandPose;
    Matrix4 rightHandPose;
    while(running) {
        vr::VRCompositor()->WaitGetPoses(devicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
        int hand = 0;
        for (int deviceIdx = 0; deviceIdx < vr::k_unMaxTrackedDeviceCount; ++deviceIdx) {
            if (!devicePose[deviceIdx].bPoseIsValid) {
                continue;
            }
            devicePoseMat[deviceIdx] = steamMatToMatrix4(devicePose[deviceIdx].mDeviceToAbsoluteTracking);
            if (hmd->GetTrackedDeviceClass(deviceIdx) == vr::TrackedDeviceClass_Controller) {
                if (hand == 0) {
                    rightHandPose = devicePoseMat[deviceIdx];
                    std::cout << "right hand:\n" << rightHandPose;
                }
                if (hand == 1) {
                    leftHandPose = devicePoseMat[deviceIdx];
                    std::cout << "left hand:\n" << leftHandPose;
                }
                hand++;
            }
        }
    }

    // Cleanup
    std:: cout << "exit!\n";
    if( hmd ) {
        vr::VR_Shutdown();
    }

    return 0;
}
