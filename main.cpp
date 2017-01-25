#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <iostream>
#include <vector>
#include <GL/glew.h>

#include "openvr.h"

#include "geom/Matrices.h"

using std::cout;
using std::vector;
using vr::EVRInitError;
using vr::IVRSystem;
using vr::TrackedDevicePose_t;
using vr::VRCompositor;
using vr::VRInitError_None;
using vr::VR_Init;
using vr::VR_GetVRInitErrorAsEnglishDescription;
using vr::VRApplication_Scene;
using vr::k_unMaxTrackedDeviceCount;

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

static bool initGl() {
    int nWindowPosX = 700;
    int nWindowPosY = 100;
    Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

    auto m_pCompanionWindow = SDL_CreateWindow( "hellovr", nWindowPosX, nWindowPosY, 400, 400, unWindowFlags );
    if (m_pCompanionWindow == NULL)
    {
        printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    auto m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
    return true;
}

int main() {
    signal(SIGINT, sig_handler);

    if(!initGl())
        return 0;

    // Init HMD
    EVRInitError eError = VRInitError_None;
    IVRSystem *hmd = VR_Init(&eError, VRApplication_Scene);
    if (eError != VRInitError_None) {
        printf("Unable to init VR runtime: %s", VR_GetVRInitErrorAsEnglishDescription(eError));
        return -1;
    }
    if (!VRCompositor()) {
        printf("Compositor initialization failed. See log file for details\n", __FUNCTION__);
        return -1;
    }

    // Init glew
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(glewError));
        return false;
    }
    glGetError(); // to clear the error caused deep in GLEW

    // Main loop
    TrackedDevicePose_t devicePose[ k_unMaxTrackedDeviceCount ];
    Matrix4 devicePoseMat[ k_unMaxTrackedDeviceCount ];
    Matrix4 leftHandPose;
    Matrix4 rightHandPose;
    std::vector<float> floatAr;
    while(running) {
        VRCompositor()->WaitGetPoses(devicePose, k_unMaxTrackedDeviceCount, NULL, 0);

        // Track devices
        int hand = 0;
        for (int deviceIdx = 0; deviceIdx < k_unMaxTrackedDeviceCount; ++deviceIdx) {
            const vr::TrackedDevicePose_t& pose = devicePose[deviceIdx];
            if (!pose.bPoseIsValid) {
                continue;
            }
            Matrix4 poseMat = steamMatToMatrix4(pose.mDeviceToAbsoluteTracking);
            devicePoseMat[deviceIdx] = poseMat;
            const Matrix4 &mat = devicePoseMat[deviceIdx];

            Vector4 start = mat * Vector4(0, 0, 0.0f, 1);
            Vector4 end = mat * Vector4(0, 0, -39.f, 1);
            Vector3 color(0, 0, 1);

            floatAr.push_back(start.x); floatAr.push_back(start.y); floatAr.push_back(start.z);
            floatAr.push_back(color.x); floatAr.push_back(color.y); floatAr.push_back(color.z);

            floatAr.push_back(end.x); floatAr.push_back(end.y); floatAr.push_back(end.z);
            floatAr.push_back(color.x); floatAr.push_back(color.y); floatAr.push_back(color.z);
        }

        // Render devices
        unsigned int controllerVertCount = floatAr.size() / 6;
        GLuint controllerVertAr = 0;
        GLuint controllerVertBuffer = 0;
        if (controllerVertAr == 0) {
            glGenVertexArrays(1, &controllerVertAr);
            glBindVertexArray(controllerVertAr);

            glGenBuffers(1, &controllerVertBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, controllerVertBuffer);

            GLuint stride = 2 * 3 * sizeof(float);
            GLuint offset = 0;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

            offset += sizeof(Vector3);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

            glBindVertexArray(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, controllerVertBuffer);

        // set vertex data if we have some
        if (floatAr.size() > 0) {
            //$ TODO: Use glBufferSubData for this...
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatAr.size(), &floatAr[0], GL_STREAM_DRAW);
        }

    }

    // Cleanup
    cout << "exit!\n";
    if( hmd ) {
        vr::VR_Shutdown();
    }

    return 0;
}
