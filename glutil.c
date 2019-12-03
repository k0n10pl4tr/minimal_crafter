#include "glutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glad.h"
#include "util.h"



unsigned int
loadShader(const char *path, unsigned int type)
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
		printf("Shader compilation failure! Shader path: %s\nLog: %s\n", path, data);
		free(data);
	}
	
	free(fileData);
	return shader;
}

unsigned int
createProgram(unsigned int shaders[])
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
		shaderPtr++;
	}

	return program;
}

unsigned int
loadTextureFarbfeld(const char *path)
{
	char header[8];
	unsigned int w, h;
	unsigned short *imageData;
	
	FILE *fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open the image: %s\n", path);
		return -1;
	}

	fread(header, sizeof(header), 1, fp);
	if(strcmp(header, "farbfeld") == 0) {
		printf("This is not a farbfeld image: %s\n", path);
		return -2;
	}

	fread(&w, sizeof(w), 1, fp);
	fread(&h, sizeof(h), 1, fp);

	w = bigEndianToHost32(w);
	h = bigEndianToHost32(h);

	imageData = malloc(4 * w * h * sizeof(short));
	fread(imageData, 1, sizeof(short) * w * h * 4, fp);
	
	for(unsigned int i = 0; i < w * h * 4; i++) {
		imageData[i] = bigEndianToHost16(imageData[i]);
	}

	fclose(fp);

	unsigned int texture;
	glGenTextures(1, &texture); 
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_SHORT, imageData);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	free(imageData);
	return texture;
}
