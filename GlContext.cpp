#include "GlContext.h"

GlContext::GlContext() {
}

GlContext::~GlContext() {
}

bool GlContext::init() {
	return true;
}

bool GlContext::createFrameBuffer(int width, int height, FramebufferDesc &framebufferDesc) {
	glGenFramebuffers(1, &framebufferDesc.renderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.renderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.depthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.depthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.depthBufferId);

	glGenTextures(1, &framebufferDesc.renderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.renderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
		framebufferDesc.renderTextureId, 0);

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

GLuint GlContext::compileGlShader(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader) {
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader(nSceneVertexShader);

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE) {
		char buffer[1024];
		GLsizei len = 0;
		glGetShaderInfoLog(nSceneVertexShader, 1024, &len, buffer);
		printf("%s - Unable to compile vertex shader %d!\n %s \n", pchShaderName, nSceneVertexShader, buffer);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneVertexShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneVertexShader);
	glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

	GLuint nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader(nSceneFragmentShader);

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE) {
		char buffer[1024];
		GLsizei len = 0;
		glGetShaderInfoLog(nSceneFragmentShader, 1024, &len, buffer);
		printf("%s - Unable to compile fragment shader %d!\n %s \n", pchShaderName, nSceneFragmentShader, buffer);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneFragmentShader);
		return 0;
	}

	glAttachShader(unProgramID, nSceneFragmentShader);
	glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

	glLinkProgram(unProgramID);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(unProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE) {
		printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram(unProgramID);
		return 0;
	}

	glUseProgram(unProgramID);
	glUseProgram(0);

	return unProgramID;
}
