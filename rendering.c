#include "rendering.h"

#include "glad.h"
#include "glutil.h"
#include "linmath.h"

#include <stdio.h>

unsigned int cubeRenderingShader = 0;
unsigned int cubeVertexArray = 0;
unsigned int cubeVertexBuffer = 0;

unsigned int cubeUniformProjectionLocation = 0;
unsigned int cubeUniformModelLocation      = 0;
unsigned int cubeUniformViewLocation       = 0;

mat4x4 projectionMatrix;
mat4x4 modelMatrix;
mat4x4 viewMatrix;

#define TO_RADIANS(x) (x * M_PI) / 180.0

const float cubeBufferData[] = {
	//Back face
	-1, -1, -1,
	 1, -1, -1,
	 1,  1, -1, 
	 1,  1, -1,
	-1,  1, -1, 
	-1, -1, -1,

	//Left face
	-1, -1, -1,
	-1, -1,  1,
	-1,  1,  1,
	-1,  1,  1,
	-1,  1, -1,
	-1, -1, -1,

	//Front face
	-1, -1,  1,
	-1,  1,  1,
	 1,  1,  1, 
	 1,  1,  1,
	 1, -1,  1, 
	-1, -1,  1,

	//Right face
	-1, -1, -1,
	-1,  1, -1,
	-1,  1,  1,
	-1,  1,  1,
	-1, -1,  1,
	-1, -1, -1,
	
	//Bottom face
	-1, -1, -1,
	 1, -1, -1,
	 1, -1,  1,
	 1, -1,  1,
	-1, -1,  1,
	-1, -1, -1,
	
	//Top face
	-1,  1, -1,
	-1,  1,  1,
	 1,  1,  1,
	 1,  1,  1,
	 1,  1, -1,
	-1,  1, -1
};

vec3 eyeDirection = { 0.0, 0.0, 1.0 };
vec3 eyePosition  = { 0.0, 0.0, -10.0 };
vec3 eyeUpNormal  = { 0.0, 1.0, 0.0 };

void
initRenderingSystem()
{
	unsigned int cubeRenderingVertex = loadShader("shaders/cubeRendering.vsh", GL_VERTEX_SHADER);
	unsigned int cubeRenderingFragment = loadShader("shaders/cubeRendering.fsh", GL_FRAGMENT_SHADER);

	unsigned int cubeRenderingShaderShaders[] = { cubeRenderingVertex, cubeRenderingFragment, 0 };
	cubeRenderingShader = createProgram(cubeRenderingShaderShaders);

	glDeleteShader(cubeRenderingVertex);
	glDeleteShader(cubeRenderingFragment);

	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeBufferData), cubeBufferData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &cubeVertexArray);
	glBindVertexArray(cubeVertexArray);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	cubeUniformProjectionLocation = glGetUniformLocation(cubeRenderingShader, "projectionMatrix");
	cubeUniformModelLocation      = glGetUniformLocation(cubeRenderingShader, "modelMatrix");
	cubeUniformViewLocation       = glGetUniformLocation(cubeRenderingShader, "viewMatrix");
	
	mat4x4_perspective(projectionMatrix, TO_RADIANS(70.0), 4.0/3.0, 0.01, 100.0);
	mat4x4_identity(modelMatrix);
	mat4x4_look_at(viewMatrix, eyeDirection, eyePosition, eyeUpNormal);

	glUseProgram(cubeRenderingShader);
	glUniformMatrix4fv(cubeUniformProjectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(cubeUniformModelLocation,      1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(cubeUniformViewLocation,       1, GL_FALSE, &viewMatrix[0][0]);
	glUseProgram(0);
}

void
resizeRenderingSystem(int w, int h)
{
	printf("Resizing to %dx%d (aspect: %f\n", w, h, (float)w/h);
	mat4x4_perspective(projectionMatrix, TO_RADIANS(70.0), (float)w/h, 0.01, 100.0);
	glUseProgram(cubeRenderingShader);
	glUniformMatrix4fv(cubeUniformProjectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUseProgram(0);

	glViewport(0, 0, w, h);
}

void
render()
{
	glUseProgram(cubeRenderingShader);
	glBindVertexArray(cubeVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
}


