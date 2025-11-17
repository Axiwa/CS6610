#include <iostream>
#include "cy/cyTriMesh.h"
#include "cy/cyMatrix.h"
#include "cy/cyVector.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "plane.h"
#include "lodepng.h"

#define M_PI 3.1415926

struct VertexModel {
	cyVec3f pos;
	cyVec3f normal;
	cyVec3f st;
};

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
	std::vector<VertexModel> modelvertices(mesh.NV());
	// Align indecies
	for (int i = 0; i < mesh.NF(); i++) {
		cyTriMesh::TriFace vertIndex = mesh.F(i);
		cyTriMesh::TriFace normalIndex = mesh.FN(i);
		cyTriMesh::TriFace stIndex = mesh.FT(i);

		VertexModel v;
		v.pos = mesh.V(vertIndex.v[0]);
		v.normal = mesh.VN(normalIndex.v[0]);
		v.st = mesh.VT(stIndex.v[0]);
		modelvertices[vertIndex.v[0]] = v;

		v.pos = mesh.V(vertIndex.v[1]);
		v.normal = mesh.VN(normalIndex.v[1]);
		v.st = mesh.VT(stIndex.v[1]);
		modelvertices[vertIndex.v[1]] = v;

		v.pos = mesh.V(vertIndex.v[2]);
		v.normal = mesh.VN(normalIndex.v[2]);
		v.st = mesh.VT(stIndex.v[2]);
		modelvertices[vertIndex.v[2]] = v;
	}

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

	// Texture
	GLuint programTex = glCreateProgram();
	GLuint vertshaderTex, fragshaderTex;
	if (!LoadShaders("D:/learn/CS6610/proj5/teapot.vert", "D:/learn/CS6610/proj5/teapot.frag", vertshaderTex, fragshaderTex)) {
		std::cerr << "Failed to load shaders!" << std::endl;
		exit(1);
	}
	glAttachShader(programTex, vertshaderTex);
	glAttachShader(programTex, fragshaderTex);
	glLinkProgram(programTex);
	glGetProgramiv(programTex, GL_LINK_STATUS, (int*)&success);
	if (success == GL_FALSE) {
		glGetProgramiv(programTex, GL_INFO_LOG_LENGTH, (int*)&maxLength);

		char* shaderProgramInfoLog = (char*)malloc(maxLength);
		glGetProgramInfoLog(programTex, maxLength, &maxLength, shaderProgramInfoLog);
		std::cout << shaderProgramInfoLog << std::endl;
		free(shaderProgramInfoLog);
		exit(1);
	}

	// Default texture
	unsigned char white[4] = { 255, 255, 255, 255 };
	GLuint whiteTex;
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Get Texture from obj
	std::vector<GLuint> diffusetex;
	std::vector<GLuint> speculartex;

	std::string parent = "D:/learn/CS6610/proj5/teapot/";

	for(int i = 0; i< mesh.NM(); i++){
		cyTriMesh::Mtl m = mesh.M(i);
		diffusetex.push_back(0);
		speculartex.push_back(0);

		std::vector<unsigned char> diffusetexbuffer;
		std::vector<unsigned char> speculartexbuffer;
		std::vector<unsigned char> diffusedata;
		std::vector<unsigned char> speculardata;

		std::string map_kd = m.map_Kd;
		unsigned int width_d, height_d;
		lodepng::load_file(diffusetexbuffer, parent + map_kd);
		lodepng::decode(diffusedata, width_d, height_d, diffusetexbuffer);
		if (diffusedata.size()) {
			GLuint d;
			glGenTextures(1, &d);
			glBindTexture(GL_TEXTURE_2D, d);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_d, height_d, 0, GL_RGBA, GL_UNSIGNED_BYTE, diffusedata.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
			diffusetex[i] = d;
		}

		std::string map_ks = m.map_Ks;
		unsigned int width_s, height_s;
		lodepng::load_file(speculartexbuffer, parent + map_ks);
		lodepng::decode(speculardata, width_s, height_s, speculartexbuffer);
		if (speculardata.size()) {
			GLuint s;
			glGenTextures(1, &s);
			glBindTexture(GL_TEXTURE_2D, s);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_s, height_s, 0, GL_RGBA, GL_UNSIGNED_BYTE, speculardata.data());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
			speculartex[i] = s;
		}
	}

	// Create a framebuffer
	GLuint frameBuffer = 0;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	GLuint renderTexture;
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // Allocate this space for me!!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 1024);

	// Configure frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); // You have to bind before configuration
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture, 0);

	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		std::cerr << "Frame buffer is not configured!" << std::endl;
		exit(1);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Plane
	GLuint planeVAO, planeVBO, planeEBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	GLuint pos = glGetAttribLocation(program, "pos");
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(pos);

	GLuint coord = glGetAttribLocation(program, "in_tex");
	glVertexAttribPointer(coord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, s)));
	glEnableVertexAttribArray(coord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Texture
	GLuint modelVAO, modelVBO, modelEBO;
	glGenVertexArrays(1, &modelVAO);
	glGenBuffers(1, &modelVBO);
	glGenBuffers(1, &modelEBO);

	glBindVertexArray(modelVAO);

	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, modelvertices.size() * sizeof(VertexModel), modelvertices.data(), GL_STATIC_DRAW);

	GLuint posTex = glGetAttribLocation(programTex, "pos");
	glVertexAttribPointer(posTex, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)0);
	glEnableVertexAttribArray(posTex);

	GLuint normalTex = glGetAttribLocation(programTex, "in_normal");
	glVertexAttribPointer(normalTex, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)offsetof(VertexModel, normal));
	glEnableVertexAttribArray(normalTex);

	GLuint corrdTex = glGetAttribLocation(programTex, "in_tex");
	glVertexAttribPointer(corrdTex, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)offsetof(VertexModel, st));
	glEnableVertexAttribArray(corrdTex);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.NF() * sizeof(unsigned int) * 3, &mesh.F(0), GL_STATIC_DRAW);

	glBindVertexArray(0);

	cy::Matrix4f model = cy::Matrix4f::Identity();
	cy::Matrix4f view = cy::Matrix4f::Identity();
	cy::Matrix4f proj = cy::Matrix4f::Identity();
	cyMatrix4f rotation = cy::Matrix4f::Identity();

	cy::Matrix4f modelTex = cy::Matrix4f::Identity();
	modelTex.Zero();
	modelTex.cell[2] = 1;
	modelTex.cell[4] = 1;
	modelTex.cell[9] = 1;
	modelTex.cell[15] = 1;
	cy::Matrix4f viewTex = cy::Matrix4f::Identity();
	cy::Matrix4f projTex = cy::Matrix4f::Identity();
	cyMatrix4f rotationTex = cy::Matrix4f::Identity();

	float phi = 0, theta = M_PI / 2, fixed = 10;
	float phiTex = 0, thetaTex = M_PI / 2, distTex = 50;
	float philight = 0, thetalight = M_PI / 2;

	while (!glfwWindowShouldClose(mywindow->window)) {
		// Transformation
		double deltaX, deltaY, zoom, deltaXTex = 0, deltaYTex = 0;
		mywindow->deltaMouse(deltaX, deltaY);

		// Plane zoom
		float distance = fixed * mywindow->zoom;

		if (glfwGetKey(mywindow->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			philight += deltaX / mywindow->width * 2 * M_PI;
			thetalight += deltaY / mywindow->height * 2 * M_PI;
			deltaX = 0;
			deltaY = 0;
		}
		else if (glfwGetKey(mywindow->window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) { // RenderTexture
			if (mywindow->isDraggingLeft) {
				deltaXTex = deltaX;
				deltaYTex = deltaY;
			}
			else if (mywindow->isDraggingRight) {
				if (deltaX > 0) {
					distTex /= 1.1;
				}
				else if (deltaX < 0) {
					distTex *= 1.1;
				}
				distTex = std::max(0.1f, std::min(distTex, 1000.0f));
			}
			deltaX = 0;
			deltaY = 0;
		}
		else { // Plane
			deltaXTex = 0;
			deltaYTex = 0;
		}

		float lightx = sin(thetalight) * sin(philight);
		float lighty = cos(thetalight);
		float lightz = sin(thetalight) * cos(philight);

		// MVP of mesh
		glUseProgram(programTex);
		glBindVertexArray(modelVAO);

		GLint origFB;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glViewport(0, 0, 1024, 1024);
		glClearColor(0.2, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaPhiTex = deltaXTex / mywindow->width * 2 * M_PI;
		float deltaThetaTex = deltaYTex / mywindow->height * 2 * M_PI;
		phiTex -= deltaPhiTex;
		thetaTex -= deltaThetaTex;

		float sinphiTex = sin(phiTex);
		float sinthetaTex = sin(thetaTex);
		float cosphiTex = cos(phiTex);
		float costhetaTex = cos(thetaTex);
		cyMatrix4f rt1 = cyMatrix4f::Identity().RotationY(deltaPhiTex);
		cyMatrix4f rt2 = cyMatrix4f::Identity().RotationX(deltaThetaTex);
		rotationTex = rt2 * rotationTex * rt1;
		float eyex = sinthetaTex * sinphiTex * distTex;
		float eyey = costhetaTex * distTex;
		float eyez = sinthetaTex * cosphiTex * distTex;
		cyMatrix4f translateTex = cyMatrix4f::Translation({ -eyex, -eyey, -eyez });

		viewTex = rotationTex * translateTex;
		projTex.SetPerspective(M_PI / 2, mywindow->width * 1.0 / mywindow->height, 0.1, 1000);

		for (int i = 0; i < mesh.NM(); i++) {
			int modellocTex = glGetUniformLocation(programTex, "model");
			int viewlocTex = glGetUniformLocation(programTex, "view");
			int projlocTex = glGetUniformLocation(programTex, "proj");
			glUniformMatrix4fv(modellocTex, 1, GL_FALSE, &modelTex.cell[0]);
			glUniformMatrix4fv(viewlocTex, 1, GL_FALSE, &viewTex.cell[0]);
			glUniformMatrix4fv(projlocTex, 1, GL_FALSE, &projTex.cell[0]);

			int eyelocTex = glGetUniformLocation(programTex, "in_eye");
			glUniform3f(eyelocTex, eyex, eyey, eyez);
			int lightlocTex = glGetUniformLocation(programTex, "in_light");
			glUniform3f(lightlocTex, lightx, lighty, lightz);

			if (diffusetex[i] > 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, diffusetex[i]);
				GLuint sampler0 = glGetUniformLocation(program, "basecolorTex");
				glUniform1i(sampler0, 0);
			}
			else {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, whiteTex);
				GLuint sampler0 = glGetUniformLocation(program, "basecolorTex");
				glUniform1i(sampler0, 0);
			}
			if (speculartex[i] > 0) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, speculartex[i]);
				GLuint sampler1 = glGetUniformLocation(program, "specularcolorTex");
				glUniform1i(sampler1, 1);
			}
			else {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, whiteTex);
				GLuint sampler1 = glGetUniformLocation(program, "specularcolorTex");
				glUniform1i(sampler1, 1);
			}

			glDrawElements(GL_TRIANGLES, mesh.NF() * 3, GL_UNSIGNED_INT, nullptr);
		}

		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, renderTexture);
		glGenerateMipmap(GL_TEXTURE_2D);

		// MVP of Plane
	 	glUseProgram(program);
		glBindVertexArray(planeVAO);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFB);
		glViewport(0, 0, mywindow->width, mywindow->height);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float deltaPhi = deltaX / mywindow->width * 2 * M_PI;
		float deltaTheta = deltaY / mywindow->height * M_PI;
		phi -= deltaPhi;
		theta -= deltaTheta;

		float sinphi = sin(phi);
		float sintheta = sin(theta);
		float cosphi = cos(phi);
		float costheta = cos(theta);

		cyMatrix4f r1 = cyMatrix4f::Identity().RotationY(deltaPhi);
		cyMatrix4f r2 = cyMatrix4f::Identity().RotationX(deltaTheta);
		rotation = r2 * rotation * r1;

		view.SetTranslation({ -sintheta * sinphi * distance, -costheta * distance, -sintheta * cosphi * distance });
		view = rotation * view;
		proj.SetPerspective(M_PI / 2, mywindow->width * 1.0 / mywindow->height, 0.1, 1000);

		int modelloc = glGetUniformLocation(program, "model");
		glUniformMatrix4fv(modelloc, 1, GL_FALSE, &model.cell[0]);
		int viewloc = glGetUniformLocation(program, "view");
		glUniformMatrix4fv(viewloc, 1, GL_FALSE, &view.cell[0]);
		int projloc = glGetUniformLocation(program, "proj");
		glUniformMatrix4fv(projloc, 1, GL_FALSE, &proj.cell[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderTexture);
		GLuint sampler = glGetUniformLocation(program, "rendertexture");
		glUniform1i(sampler, 0);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		glfwSwapBuffers(mywindow->window);
		glfwPollEvents();
	}

	glfwDestroyWindow(mywindow->window);
	delete(mywindow);
	glfwTerminate();
}