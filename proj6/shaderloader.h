#pragma once

#include <iostream>
#include <glad/glad.h>

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

	int success;
	int maxLength;
	glCompileShader(shader);
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