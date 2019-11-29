#include "glutil.h"

#include <stdio.h>
#include <stdlib.h>

#include "glad.h"

unsigned int
glutil_load_shader(const char *path, unsigned int type)
{
	char *fileData;
	int   size;

	FILE *fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open the shader %s\n", path);
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileData = malloc(size);
	fread(fileData, 1, size, fp);
	fclose(fp);

	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char* const*)&fileData, &size);
	glCompileShader(shader);
	
	int cmp;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &cmp);
	if(!cmp) 
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &cmp);
		char* data = malloc(cmp);
		glGetShaderInfoLog(shader, cmp, &cmp, data);
		printf("Shader compilation failure! Shader path: %d\nLog: %s\n", path, data);
		free(data);
	}
	
	free(fileData);
	return shader;
}

unsigned int
glutil_create_program(unsigned int shaders[])
{
	unsigned int* shaderPtr = shaders;
	unsigned int program = glCreateProgram();

	while(*shaderPtr) {
		glAttachShader(program, *shaderPtr);
		shaderPtr++;
	}

	glLinkProgram(program);
	shaderPtr = shaders;

	while(*shaderPtr) {
		glDetachShader(program, *shaderPtr);
	}
	return program;
}

