#include "Square.h"

using namespace std;

Square::Square() {
	shader = 0;
	shaderMatrix = 0;
	vertexArray = 0;
	vertexBuffer = 0;
}

Square::~Square() {
}

void Square::init() {
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
			throw new exception("Unable to find matrix uniform in controller shader\n");
		}

		vector<float> floatAr;
		Vector3 color(1, 1, 1);

		floatAr.push_back(0);
		floatAr.push_back(0);
		floatAr.push_back(0);
		floatAr.push_back(color.x);
		floatAr.push_back(color.y);
		floatAr.push_back(color.z);

		floatAr.push_back(1);
		floatAr.push_back(0);
		floatAr.push_back(0);
		floatAr.push_back(color.x);
		floatAr.push_back(color.y);
		floatAr.push_back(color.z);

		floatAr.push_back(1);
		floatAr.push_back(0);
		floatAr.push_back(1);
		floatAr.push_back(color.x);
		floatAr.push_back(color.y);
		floatAr.push_back(color.z);

		floatAr.push_back(0);
		floatAr.push_back(0);
		floatAr.push_back(1);
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

void Square::render(Matrix4 proj) {
	glUseProgram(shader);
	glUniformMatrix4fv(shaderMatrix, 1, GL_FALSE, (proj).get());
	glBindVertexArray(vertexArray);
	glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
	glBindVertexArray(0);
	glUseProgram(0);
}



