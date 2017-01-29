#include "Controller.h"

using namespace std;

Controller::Controller() {
	controllerShader = 0;
	controllerShaderMatrix = 0;
	controllerVertAr = 0;
	controllerVertBuffer = 0;
}

bool Controller::init() { // TODO: Better pattern than init()
	if (controllerShader == 0) { // TODO: Thread safety?
		controllerShader = GlContext::compileGlShader( // TODO: Shaders as resources
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
		controllerShaderMatrix = glGetUniformLocation(controllerShader, "matrix");
		if (controllerShaderMatrix == -1) {
			printf("Unable to find matrix uniform in controller shader\n");
			return false;
		}

		vector<float> floatAr;
		Vector4 start = Vector4(0, 0, 0.0f, 1);
		Vector4 end = Vector4(0, 0, -39.f, 1);
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

		// Generate VBs for devices
		controllerVertCount = floatAr.size() / 6;
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

		glBindBuffer(GL_ARRAY_BUFFER, controllerVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatAr.size(), &floatAr[0], GL_STREAM_DRAW);
	}
	return true;
}

Controller::~Controller() {
	// TODO: cleanup
}

void Controller::setPose(Matrix4 pose) {
	this->pose = pose;
}

void Controller::render(Matrix4 proj) {
	glUseProgram(controllerShader);
	glUniformMatrix4fv(controllerShaderMatrix, 1, GL_FALSE, (proj * pose).get());
	glBindVertexArray(controllerVertAr);
	glDrawArrays(GL_LINES, 0, controllerVertCount);
	glBindVertexArray(0);
	glUseProgram(0);
}
