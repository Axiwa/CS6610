#include <iostream>
#include "cy/cyTriMesh.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "plane.h"

#define M_PI 3.1415926

struct myWindow {
	myWindow():
		width(1024),
		height(1024),
		mouseX(0.0),
		mouseY(0.0),
		window(nullptr),
		zoom(1)
	{}
	myWindow(int width_, int height_, const char* title):
		width(width_),
		height(height_),
		mouseX(0.0),
		mouseY(0.0),
		window(nullptr),
		zoom(1)
	{
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetScrollCallback(window, ScrollCallback);
	}
	void deltaMouse(double& deltax, double& deltay) {
		if (isDraggingLeft || isDraggingRight) {
			deltax = mouseX - lastX;
			deltay = mouseY - lastY;
			lastX = mouseX;
			lastY = mouseY;
		}
		else {
			deltax = 0;
			deltay = 0;
		}
	}
	GLFWwindow* window;
	int width, height;
	double mouseX, mouseY, lastX = 0, lastY = 0;
	bool isDraggingLeft = false, isDraggingRight = false;
	double zoom;

private:
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
		myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
		self->width = width;
		self->height = height;
		glViewport(0, 0, width, height);
	}
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
		self->mouseX = xpos;
		self->mouseY = ypos;
	}
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				self->isDraggingLeft = true;
				self->lastX = self->mouseX;
				self->lastY = self->mouseY;
			}
			else {
				self->isDraggingLeft = false;
			}
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				self->isDraggingRight = true;
				self->lastX = self->mouseX;
				self->lastY = self->mouseY;
			}
			else {
				self->isDraggingRight = false;
			}
		}
	}
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		myWindow* self = static_cast<myWindow*>(glfwGetWindowUserPointer(window));
		if (yoffset < 0) {
			self->zoom *= 1.1f;
		}
		else {
			self->zoom /= 1.1f;
		}
	}
};

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;

	fptr = fopen(file, "rb"); /* Open file for reading */
	if (!fptr) /* Return NULL on failure */
		return NULL;
	fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
	length = ftell(fptr); /* Find out how many bytes into the file we are */
	buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
	fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
	fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
	fclose(fptr); /* Close the file */
	buf[length] = 0; /* Null terminator */

	return buf; /* Return the buffer */
}

bool LoadShaders(const char* vertfile, const char* fragfile, GLuint& vertshader, GLuint& fragshader) {
	if (vertshader != 0) {
		glDeleteShader(vertshader);
	}
	if (fragshader != 0) {
		glDeleteShader(fragshader);
	}
    vertshader = glCreateShader(GL_VERTEX_SHADER);
    fragshader = glCreateShader(GL_FRAGMENT_SHADER);

	GLchar* vertsource = filetobuf(vertfile);
	GLchar* fragsource = filetobuf(fragfile);

	glShaderSource(vertshader, 1, (const GLchar**)&vertsource, 0);
	glShaderSource(fragshader, 1, (const GLchar**)&fragsource, 0);

	int success;
	int maxLength;

	glCompileShader(vertshader);
	glGetShaderiv(vertshader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderiv(vertshader, GL_INFO_LOG_LENGTH, &maxLength);
		char* vertexInfoLog;

		/* The maxLength includes the NULL character */
		vertexInfoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(vertshader, maxLength, &maxLength, vertexInfoLog);
		std::cout << vertexInfoLog << std::endl;
		free(vertexInfoLog);
		return false;
	}
	glCompileShader(fragshader);
	glGetShaderiv(fragshader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderiv(fragshader, GL_INFO_LOG_LENGTH, &maxLength);
		char* fragInfoLog;

		/* The maxLength includes the NULL character */
		fragInfoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(fragshader, maxLength, &maxLength, fragInfoLog);
		std::cout << fragInfoLog << std::endl;
		free(fragInfoLog);
		return false;
	}

	return true;
}

int main(int argc, char** argv) {

	cyTriMesh mesh;
	mesh.LoadFromFileObj("D:/learn/CS6610/proj5/teapot/teapot.obj");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	myWindow* mywindow = new myWindow(1024, 1024, "Hiiii");
	glfwMakeContextCurrent(mywindow->window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	GLuint program = glCreateProgram();
	GLuint vertshader, fragshader;
	if (!LoadShaders("D:/learn/CS6610/proj5/plane.vert", "D:/learn/CS6610/proj5/plane.frag", vertshader, fragshader)) {
		std::cerr << "Failed to load shaders!" << std::endl;
		exit(1);
	}
	glAttachShader(program, vertshader);
	glAttachShader(program, fragshader);
	glLinkProgram(program);

	int success;
	int maxLength;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&success);
	if (success == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, (int*)&maxLength);
		
		char* shaderProgramInfoLog = (char*)malloc(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, shaderProgramInfoLog);
		std::cout << shaderProgramInfoLog << std::endl;
		free(shaderProgramInfoLog);
		exit(1);
	}

	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);
	GLuint pos = glGetAttribLocation(program, "pos");
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);
	glEnableVertexAttribArray(pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	cy::Matrix4f model = cy::Matrix4f::Identity();
	cy::Matrix4f view = cy::Matrix4f::Identity();
	cy::Matrix4f proj = cy::Matrix4f::Identity();
	cyMatrix4f rotation = cy::Matrix4f::Identity();

	float phi = 0, theta = M_PI / 2, fixed = 10;

	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(mywindow->window)) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float distance = fixed * mywindow->zoom;

		// Plane transformation
		double deltaX, deltaY;
		mywindow->deltaMouse(deltaX, deltaY);

		glUseProgram(program);
		glBindVertexArray(VAO);

		// MVP
		float deltaPhi = deltaX / mywindow->width * 2 * M_PI;
		float deltaTheta = deltaY / mywindow->height * M_PI;
		phi -= deltaPhi;
		theta -= deltaTheta;

		float sinphi = sin(phi);
		float sintheta = sin(theta);
		float cosphi = cos(phi);
		float costheta = cos(theta);
		view.SetTranslation({ -sintheta * sinphi * distance, -costheta * distance, -sintheta * cosphi * distance });

		cyMatrix4f r1 = cyMatrix4f::Identity().RotationY(deltaPhi);
		cyMatrix4f r2 = cyMatrix4f::Identity().RotationX(deltaTheta);
		rotation = r2 * rotation * r1;
		
		view = rotation * view;

		proj.SetPerspective(M_PI / 2, mywindow->width * 1.0 / mywindow->height, 0.1, 1000);

		int modelloc = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelloc, 1, GL_FALSE, &model.cell[0]);
		int viewloc = glGetUniformLocation(program, "view");
		glUniformMatrix4fv(viewloc, 1, GL_FALSE, &view.cell[0]);
		int projloc = glGetUniformLocation(program, "proj");
		glUniformMatrix4fv(projloc, 1, GL_FALSE, &proj.cell[0]);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		glfwSwapBuffers(mywindow->window);
		glfwPollEvents();
	}

	glfwDestroyWindow(mywindow->window);
	delete(mywindow);
	glfwTerminate();
}