#include <iostream>

#include "openvr.h"

int main() {
    vr::EVRInitError eError = vr::VRInitError_None;
    vr::IVRSystem *hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None) {
        printf("Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
        return false;
    }
    vr::ETrackedPropertyError err;
    for (uint32_t deviceIdx = vr::k_unTrackedDeviceIndex_Hmd + 1; deviceIdx < vr::k_unMaxTrackedDeviceCount; deviceIdx++)	{
        if (!hmd->IsTrackedDeviceConnected(deviceIdx))
            continue;
        uint32_t buffLen = hmd->GetStringTrackedDeviceProperty(deviceIdx, vr::Prop_TrackingSystemName_String, NULL, 0, &err);
        if (buffLen == 0) {
            return false;
        }

        char *buff = new char[buffLen];
        buffLen = hmd->GetStringTrackedDeviceProperty(deviceIdx, vr::Prop_TrackingSystemName_String, buff, buffLen, &err);
        std::string sResult = buff;
        delete[] buff;
        std::cout << sResult;
    }

    return 0;
}