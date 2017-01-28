#include <iostream>
#include <vector>
#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <GL/glew.h>

#include "openvr.h"

#include "geom/Matrices.h"

using namespace std;
using namespace vr;

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

struct VertexDataWindow {
    Vector2 position;
    Vector2 texCoord;

    VertexDataWindow(const Vector2 & pos, const Vector2 tex) : position(pos), texCoord(tex) {	}
};

struct FramebufferDesc {
    GLuint depthBufferId;
    GLuint renderTextureId;
    GLuint renderFramebufferId;
    GLuint resolveTextureId;
    GLuint resolveFramebufferId;
};

void renderPerspective(uint32_t hmdWidth, uint32_t hmdHeight, GLuint controllerShader, GLint controllerShaderMatrix,
                       const FramebufferDesc &leftEyeDesc, unsigned int controllerVertCount, GLuint controllerVertAr);

bool createFrameBuffer(int width, int height, FramebufferDesc &framebufferDesc) {
    glGenFramebuffers(1, &framebufferDesc.renderFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.renderFramebufferId);

    glGenRenderbuffers(1, &framebufferDesc.depthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.depthBufferId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.depthBufferId);

    glGenTextures(1, &framebufferDesc.renderTextureId);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.renderTextureId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.renderTextureId, 0);

    glGenFramebuffers(1, &framebufferDesc.resolveFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.resolveFramebufferId);

    glGenTextures(1, &framebufferDesc.resolveTextureId);
    glBindTexture(GL_TEXTURE_2D, framebufferDesc.resolveTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.resolveTextureId, 0);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

GLuint compileGlShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader ) {
    GLuint unProgramID = glCreateProgram();

    GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
    glCompileShader( nSceneVertexShader );

    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
    if ( vShaderCompiled != GL_TRUE)
    {
        char buffer[1024];
        GLsizei  len = 0;
        glGetShaderInfoLog(nSceneVertexShader, 1024, &len, buffer);
        printf("%s - Unable to compile vertex shader %d!\n %s \n", pchShaderName, nSceneVertexShader, buffer);
        glDeleteProgram( unProgramID );
        glDeleteShader( nSceneVertexShader );
        return 0;
    }
    glAttachShader( unProgramID, nSceneVertexShader);
    glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

    GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
    glCompileShader( nSceneFragmentShader );

    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
    if (fShaderCompiled != GL_TRUE) {
        char buffer[1024];
        GLsizei  len = 0;
        glGetShaderInfoLog(nSceneFragmentShader, 1024, &len, buffer);
        printf("%s - Unable to compile fragment shader %d!\n %s \n", pchShaderName, nSceneFragmentShader, buffer );
        glDeleteProgram( unProgramID );
        glDeleteShader( nSceneFragmentShader );
        return 0;
    }

    glAttachShader( unProgramID, nSceneFragmentShader );
    glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

    glLinkProgram( unProgramID );

    GLint programSuccess = GL_TRUE;
    glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
    if ( programSuccess != GL_TRUE ) {
        printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
        glDeleteProgram( unProgramID );
        return 0;
    }

    glUseProgram( unProgramID );
    glUseProgram( 0 );

    return unProgramID;
}

int main() {
    signal(SIGINT, sig_handler);

    // Init HMD
    EVRInitError eError = VRInitError_None;
    IVRSystem *hmd = VR_Init(&eError, VRApplication_Scene);
    if (eError != VRInitError_None) {
        printf("Unable to init VR runtime: %s", VR_GetVRInitErrorAsEnglishDescription(eError));
        return -1;
    }
    uint32_t hmdWidth;
    uint32_t hmdHeight;
    hmd->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);
    if (!VRCompositor()) {
        printf("Compositor initialization failed. See log file for details\n", __FUNCTION__);
        return -1;
    }

    // Setup SDL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    if (SDL_VideoInit(NULL) != 0) {
        printf("Error initializing SDL video:  %s\n", SDL_GetError());
        return 2;
    }
    int minW = INT_MAX;
    int minH = INT_MAX;
    SDL_DisplayMode current;
    for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i){
        int should_be_zero = SDL_GetCurrentDisplayMode(i, &current);
        if(should_be_zero != 0) {
            printf("Could not get display mode for video display #%d: %s", i, SDL_GetError());
        } else {
            minW = min(current.w, minW);
            minH = min(current.h, minH);
            printf("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h, current.refresh_rate);
        }
    }
    float width = (float)minW / hmdWidth;
    float height = (float)minH / hmdHeight;
    float scale = min(width, height) * 0.8f;
    int windowWidth = (int)(hmdWidth * scale);
    int windowHeight = (int)(hmdHeight * scale);
    int windowPosX = (minW - windowWidth) / 2;
    int windowPosY = (minH - windowHeight) / 2;
    Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    SDL_Window *monitorWindow = SDL_CreateWindow("hellovr", windowPosX, windowPosY, windowWidth, windowHeight, unWindowFlags);
    if (monitorWindow == NULL) {
        printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        return false;
    }
    SDL_GLContext monitorGlContext = SDL_GL_CreateContext(monitorWindow);
    if (monitorGlContext == NULL) {
        printf("%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        return false;
    }

    // Init glew
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(glewError));
        return false;
    }
    glGetError(); // to clear the error caused deep in GLEW

    // Init GL
    GLuint controllerShader = compileGlShader(
            "ControllerShader",
            "#version 410\n"
                    "\n"
                    "uniform mat4 matrix;\n"
                    "layout(location = 0) in vec4 position;\n"
                    "layout(location = 1) in vec3 v3ColorIn;\n"
                    "out vec4 v4Color;\n"
                    "void main() {\n"
                    "\tv4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
                    "\tgl_Position = matrix * position;\n"
                    "}",
            "#version 410\n"
                    "\n"
                    "in vec4 v4Color;\n"
                    "out vec4 outputColor;\n"
                    "void main() {\n"
                    "   outputColor = v4Color;\n"
                    "}"
    );
    GLuint windowShader = compileGlShader(
            "MonitorWindow",
            "#version 410 core\n"
                    "\n"
                    "layout(location = 0) in vec4 position;\n"
                    "layout(location = 1) in vec2 v2UVIn;\n"
                    "noperspective out vec2 v2UV;\n"
                    "void main() {\n"
                    "\tv2UV = v2UVIn;\n"
                    "\tgl_Position = position;\n"
                    "}",
            "#version 410 core\n"
                    "\n"
                    "uniform sampler2D mytexture;\n"
                    "noperspective in vec2 v2UV;\n"
                    "out vec4 outputColor;\n"
                    "void main() {\n"
                    "\t\toutputColor = texture(mytexture, v2UV);\n"
                    "}");
    GLint controllerShaderMatrix = glGetUniformLocation(controllerShader, "matrix");
    if (controllerShaderMatrix == -1) {
        printf("Unable to find matrix uniform in controller shader\n");
        return false;
    }

    // Monitor window quad
    std::vector<VertexDataWindow> verts;
    verts.push_back(VertexDataWindow(Vector2(-1, -1), Vector2(0, 1)));
    verts.push_back(VertexDataWindow(Vector2(1, -1), Vector2(1, 1)));
    verts.push_back(VertexDataWindow(Vector2(-1, 1), Vector2(0, 0)));
    verts.push_back(VertexDataWindow(Vector2(1, 1), Vector2(1, 0)));

    GLushort indices[] = { 0, 1, 3,   0, 3, 2};
    unsigned int windowQuadIdxSize = _countof(indices);

    GLuint windowQuadVertAr = 0;
    GLuint monitorWinVertBuff = 0;
    GLuint monitorWinIdxBuff = 0;
    glGenVertexArrays(1, &windowQuadVertAr);
    glBindVertexArray(windowQuadVertAr);

    glGenBuffers(1, &monitorWinVertBuff);
    glBindBuffer(GL_ARRAY_BUFFER, monitorWinVertBuff);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VertexDataWindow), &verts[0], GL_STATIC_DRAW);

    glGenBuffers(1, &monitorWinIdxBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, monitorWinIdxBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, windowQuadIdxSize * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

    glBindVertexArray(0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // HMD frame buffers
    FramebufferDesc leftEyeDesc;
    FramebufferDesc rightEyeDesc;
    createFrameBuffer(hmdWidth, hmdHeight, leftEyeDesc);
    createFrameBuffer(hmdWidth, hmdHeight, rightEyeDesc);

    // Main loop
    SDL_StartTextInput();
    TrackedDevicePose_t devicePose[ k_unMaxTrackedDeviceCount ];
    Matrix4 devicePoseMat[ k_unMaxTrackedDeviceCount ];
    Matrix4 leftHandPose;
    Matrix4 rightHandPose;
    while(running) {
        // SDL input
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent) != 0) {
            if (sdlEvent.type == SDL_QUIT) {
                running = false;
            } else if (sdlEvent.type == SDL_KEYDOWN) {
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE || sdlEvent.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

        // Process SteamVR events
        vr::VREvent_t event;
        while (hmd->PollNextEvent(&event, sizeof(event))) {
            if(event.eventType == vr::VREvent_TrackedDeviceActivated) {
                //initDeviceModel(event.trackedDeviceIndex);
            }
        }

        VRCompositor()->WaitGetPoses(devicePose, k_unMaxTrackedDeviceCount, NULL, 0);

        // Track devices
        int hand = 0;
        std::vector<float> floatAr;
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
            Vector3 color(1, 1, 1);

            floatAr.push_back(-0.9f); floatAr.push_back(-0.9f); floatAr.push_back(0.5f);
            floatAr.push_back(color.x); floatAr.push_back(color.y); floatAr.push_back(color.z);

            floatAr.push_back(0.9f); floatAr.push_back(0.9f); floatAr.push_back(0.5f);
            floatAr.push_back(color.x); floatAr.push_back(color.y); floatAr.push_back(color.z);
        }

        // Generate VBs for devices
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
        if (floatAr.size() > 0) {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatAr.size(), &floatAr[0], GL_STREAM_DRAW);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Render each perspective
        renderPerspective(hmdWidth, hmdHeight, controllerShader, controllerShaderMatrix, leftEyeDesc,
                          controllerVertCount, controllerVertAr);
        renderPerspective(hmdWidth, hmdHeight, controllerShader, controllerShaderMatrix, rightEyeDesc,
                          controllerVertCount, controllerVertAr);

        // Render to monitor window
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, windowWidth, windowHeight);
        glBindVertexArray(windowQuadVertAr);
        glUseProgram(windowShader);
        glBindTexture(GL_TEXTURE_2D, leftEyeDesc.resolveTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glDrawElements(GL_TRIANGLES, windowQuadIdxSize, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // Submit to HMD
        vr::Texture_t leftEyeTexture = { (void*)leftEyeDesc.resolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        vr::Texture_t rightEyeTexture = { (void*)rightEyeDesc.resolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

        // Swap
        SDL_GL_SwapWindow(monitorWindow);

        // We want to make sure the glFinish waits for the entire present to complete, not just the submission
        // of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    SDL_StopTextInput();

    // Cleanup
    cout << "exit!\n";
    if( hmd ) {
        vr::VR_Shutdown();
    }

    return 0;
}

void renderPerspective(uint32_t hmdWidth, uint32_t hmdHeight, GLuint controllerShader, GLint controllerShaderMatrix,
                       const FramebufferDesc &leftEyeDesc, unsigned int controllerVertCount, GLuint controllerVertAr) {
    glEnable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.renderFramebufferId);
    glViewport(0, 0, hmdWidth, hmdHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    Matrix4 trans;
    glUseProgram(controllerShader);
    glUniformMatrix4fv(controllerShaderMatrix, 1, GL_FALSE, trans.get());
    glBindVertexArray(controllerVertAr);
    glDrawArrays(GL_LINES, 0, controllerVertCount);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.renderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.resolveFramebufferId);
    glBlitFramebuffer(0, 0, hmdWidth, hmdHeight, 0, 0, hmdWidth, hmdHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glEnable(GL_MULTISAMPLE);
}
