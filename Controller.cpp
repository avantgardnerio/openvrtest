#include "Controller.h"

using namespace std;

Controller::Controller(Vector3 color) {
	this->color = color;

	shader = 0;
	shaderMatrix = 0;
	vertexArray = 0;
	vertexBuffer = 0;
}

void Controller::init() { 
	if (shader == 0) { // TODO: Thread safety?
		shader = GlContext::compileGlShader( // TODO: Shaders as resources
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
		shaderMatrix = glGetUniformLocation(shader, "matrix");
		if (shaderMatrix == -1) {
			throw new runtime_error("Unable to find matrix uniform in controller shader\n");
		}

		vector<float> floatAr;
		Vector4 start = Vector4(0, 0, 0.0f, 1);
		Vector4 end = Vector4(0, 0, -39.f, 1);

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
		vertexCount = floatAr.size() / 6;
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);

		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatAr.size(), &floatAr[0], GL_STREAM_DRAW);
	}
}

Controller::~Controller() {
	// TODO: cleanup
}

void Controller::setPose(Matrix4 pose) {
	this->pose = pose;
}

void Controller::render(Matrix4 eyeProj, Matrix4 headInverse, Matrix4 worldTrans) {
	glUseProgram(shader);
	glUniformMatrix4fv(shaderMatrix, 1, GL_FALSE, (eyeProj * headInverse * pose).get());
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_LINES, 0, vertexCount);
	glBindVertexArray(0);
	glUseProgram(0);
}
