#pragma once

#include <iostream>
#include <glad/glad.h>
#include "cy/cyMatrix.h"

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

bool LoadShader(const char* vertfile, GLuint& shader, GLenum shaderType){
    if (shader != 0){
        glDeleteShader(shader);
    }

    shader = glCreateShader(shaderType);
    GLchar* content = filetobuf(vertfile);
    glShaderSource(shader, 1, (const GLchar**)& content, 0);
	glCompileShader(shader);

	int success;
	int maxLength;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		char* infoLog;

		/* The maxLength includes the NULL character */
		infoLog = (char*)malloc(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, infoLog);
		std::cout << infoLog << std::endl;
		free(infoLog);
		return false;
	}

    return true;
}

struct CameraBlock{
	cyMatrix4f model;
	cyMatrix4f view;
	cyMatrix4f projection;
	cyVec3f eyepos;
	float padding;
};

struct DirectionalLightBlock{
	cyVec4f direction;
	cyVec4f radiance;
};

struct MaterialBlock{
	cyVec4f ambient;
	cyVec4f albedo;
	cyVec4f specular;
	float shininess;
	float padding[3];
};

struct UniformBufferManager {
	GLuint uniformCamera;
	GLuint uniformLight;
	GLuint uniformMaterial;
	GLuint bindingPoint = 0;

	UniformBufferManager(){
		BindCameraUniformBlock();
		BindLightUniformBlock();
		BindMaterialUniformBlock();
	}

	void BindCameraUniformBlock(){
		glGenBuffers(1, &uniformCamera);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformCamera);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformCamera);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void BindLightUniformBlock(){
		glGenBuffers(1, &uniformLight);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformLight);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DirectionalLightBlock), nullptr, GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint + 1, uniformLight);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void BindMaterialUniformBlock(){
		glGenBuffers(1, &uniformMaterial);
		glBindBuffer(GL_UNIFORM_BUFFER, uniformMaterial);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialBlock), nullptr, GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint + 2, uniformMaterial);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	template<typename T>
	void UpdateCameraUniformBlock(const T& data, size_t offset = 0){
		glBindBuffer(GL_UNIFORM_BUFFER, uniformCamera);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	template<typename T>
	void UpdateLightUniformBlock(const T& data, size_t offset = 0){
		glBindBuffer(GL_UNIFORM_BUFFER, uniformLight);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	template<typename T>
	void UpdateMaterialUniformBlock(const T& data, size_t offset = 0){
		glBindBuffer(GL_UNIFORM_BUFFER, uniformMaterial);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T), &data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

};


