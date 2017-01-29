#include <iostream>
#include <vector>
#include <SDL_video.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <SDL_events.h>
#include <GL/glew.h>
#include <signal.h>
#include <algorithm>

#include "geom/Matrices.h"

#include "VrInput.h"
#include "SdlContext.h"
#include "SdlTargetWindow.h"
#include "GlContext.h"
#include "Controller.h"

using namespace std;
using namespace vr;

bool running = true;

void sig_handler(int signum) {
    running = false;
    printf("Received signal %d\n", signum);
}

void renderPerspective(uint32_t hmdWidth, uint32_t hmdHeight, Controller controller, const FramebufferDesc &eyeDesc, Matrix4 proj) {
    glEnable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_FRAMEBUFFER, eyeDesc.renderFramebufferId);
    glViewport(0, 0, hmdWidth, hmdHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

	controller.render(proj);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, eyeDesc.renderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeDesc.resolveFramebufferId);
    glBlitFramebuffer(0, 0, hmdWidth, hmdHeight, 0, 0, hmdWidth, hmdHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glEnable(GL_MULTISAMPLE);
}

int main() {
    signal(SIGINT, sig_handler);

    // Init HMD
	VrInput vr;
	if (!vr.init()) {
		return -1;
	}

    // Setup SDL
	SdlContext sdl;
	if (!sdl.init()) {
		return -1;
	}

	// Init GL
	GlContext gl;
	if (!gl.init()) {
		return -1;
	}

	// Create letterboxed monitor window
    float scaleX = (float) sdl.getWidth() / vr.getWidth();
    float scaleY = (float) sdl.getHeight() / vr.getHeight();
    float scale = min(scaleX, scaleY) * 0.8f;
    int width = (int) (vr.getWidth() * scale);
    int height = (int) (vr.getHeight() * scale);
    int left = (sdl.getWidth() - width) / 2;
    int top = (sdl.getHeight() - height) / 2;
	SdlTargetWindow monitorWindow(left, top, width, height);
	if (!monitorWindow.init()) {
		return -1;
	}

	// Init GL
	Controller leftController;
	if (!leftController.init()) {
		return -1;
	}
	Controller rightController;
	if (!rightController.init()) {
		return -1;
	}

    Matrix4 eyeMats[2] = {vr.getEyeProjLeft(), vr.getEyeProjRight()};
    //Matrix4 eyeMats[2] = {Matrix4(), Matrix4()};

    // HMD frame buffers
    FramebufferDesc leftEyeDesc;
    FramebufferDesc rightEyeDesc;
    gl.createFrameBuffer(vr.getWidth(), vr.getHeight(), leftEyeDesc);
    gl.createFrameBuffer(vr.getWidth(), vr.getHeight(), rightEyeDesc);

    // Main loop
	VrInputState vrInputState;
    SDL_StartTextInput();
    while (running) {
        // SDL input
        SDL_Event sdlEvent;
        while (SDL_PollEvent(&sdlEvent) != 0) {
            if (sdlEvent.type == SDL_QUIT) {
                running = false;
            }
            if (sdlEvent.type == SDL_KEYDOWN) {
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE || sdlEvent.key.keysym.sym == SDLK_q) {
                    running = false;
                }
            }
        }

		// Track devices
		vr.getState(vrInputState);
		leftController.setPose(vrInputState.leftHandPose);
		rightController.setPose(vrInputState.rightHandPose);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Render each perspective
        renderPerspective(vr.getWidth(), vr.getHeight(), leftController, leftEyeDesc, eyeMats[Eye_Left] * vrInputState.headInverse);
        renderPerspective(vr.getWidth(), vr.getHeight(), leftController, rightEyeDesc, eyeMats[Eye_Right] * vrInputState.headInverse);

        // Render to monitor window
		monitorWindow.render(leftEyeDesc.resolveTextureId);

        // Submit to HMD
        Texture_t leftEyeTexture = {(void *) leftEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma};
        VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
        Texture_t rightEyeTexture = {(void *) rightEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma};
        VRCompositor()->Submit(Eye_Right, &rightEyeTexture);

        // Swap
		monitorWindow.swap();
    }

    // Cleanup
    cout << "exit!\n";

    return 0;
}

