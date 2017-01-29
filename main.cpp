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

using namespace std;
using namespace vr;

bool running = true;

void sig_handler(int signum) {
    running = false;
    printf("Received signal %d\n", signum);
}

void renderControllers(GLuint controllerShader, GLint controllerShaderMatrix, unsigned int controllerVertCount,
                       GLuint controllerVertAr, Matrix4 proj) {
    glUseProgram(controllerShader);
    glUniformMatrix4fv(controllerShaderMatrix, 1, GL_FALSE, proj.get());
    glBindVertexArray(controllerVertAr);
    glDrawArrays(GL_LINES, 0, controllerVertCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void renderPerspective(uint32_t hmdWidth, uint32_t hmdHeight, GLuint controllerShader, GLint controllerShaderMatrix,
                       const FramebufferDesc &eyeDesc, unsigned int controllerVertCount, GLuint controllerVertAr,
                       Matrix4 proj) {
    glEnable(GL_MULTISAMPLE);
    glBindFramebuffer(GL_FRAMEBUFFER, eyeDesc.renderFramebufferId);
    glViewport(0, 0, hmdWidth, hmdHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    renderControllers(controllerShader, controllerShaderMatrix, controllerVertCount, controllerVertAr, proj);

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
	GLuint controllerShader = gl.compileGlShader(
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
    GLint controllerShaderMatrix = glGetUniformLocation(controllerShader, "matrix");
    if (controllerShaderMatrix == -1) {
        printf("Unable to find matrix uniform in controller shader\n");
        return false;
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

        vector<float> floatAr;
        for (int deviceIdx = 0; deviceIdx < 2; ++deviceIdx) {
			Matrix4 pose = deviceIdx == 0 ? vrInputState.leftHandPose : vrInputState.rightHandPose;
			Vector4 start = pose * Vector4(0, 0, 0.0f, 1);
			Vector4 end = pose * Vector4(0, 0, -39.f, 1);
			Vector3 color(1, 1, 1);

			floatAr.push_back(start.x);
			floatAr.push_back(start.y);
			floatAr.push_back(start.z);
			floatAr.push_back(color.x);
			floatAr.push_back(color.y);
			floatAr.push_back(color.z);

			floatAr.push_back(end.x);
			floatAr.push_back(end.y);
			floatAr.push_back(end.z);
			floatAr.push_back(color.x);
			floatAr.push_back(color.y);
			floatAr.push_back(color.z);
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
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *) offset);

            offset += sizeof(Vector3);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *) offset);

            glBindVertexArray(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, controllerVertBuffer);
        if (floatAr.size() > 0) {
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatAr.size(), &floatAr[0], GL_STREAM_DRAW);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Render each perspective
        renderPerspective(vr.getWidth(), vr.getHeight(), controllerShader, controllerShaderMatrix, leftEyeDesc,
                          controllerVertCount, controllerVertAr, eyeMats[Eye_Left] * vrInputState.headInverse);
        renderPerspective(vr.getWidth(), vr.getHeight(), controllerShader, controllerShaderMatrix, rightEyeDesc,
                          controllerVertCount, controllerVertAr, eyeMats[Eye_Right] * vrInputState.headInverse);

        // Render to monitor window
		monitorWindow.render(leftEyeDesc.resolveTextureId);

        // Submit to HMD
        Texture_t leftEyeTexture = {(void *) leftEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma};
        VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
        Texture_t rightEyeTexture = {(void *) rightEyeDesc.resolveTextureId, TextureType_OpenGL, ColorSpace_Gamma};
        VRCompositor()->Submit(Eye_Right, &rightEyeTexture);

        // Swap
		monitorWindow.swap();

        // We want to make sure the glFinish waits for the entire present to complete, not just the submission
        // of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    SDL_StopTextInput();

    // Cleanup
    cout << "exit!\n";

    return 0;
}

