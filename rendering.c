#include "rendering.h"

#include "glad.h"
#include "linmath.h"

#include "glutil.h"
#include "world.h"

#include <stdio.h>

unsigned int cubeRenderingShader = 0;
unsigned int cubeVertexArray = 0;
unsigned int cubeVertexBuffer = 0;

unsigned int cubeUniformProjectionLocation = 0;
unsigned int cubeUniformModelLocation      = 0;
unsigned int cubeUniformViewLocation       = 0;

unsigned int worldChunkBuffer      = 0;
unsigned int worldChunkFaces = 0;
unsigned int worldChunkVao   = 0;

unsigned int cubeTexture = 0;

mat4x4 projectionMatrix;
mat4x4 modelMatrix;
mat4x4 viewMatrix;

#define TO_RADIANS(x) (x * M_PI) / 180.0

const float cubeBufferData[] = {
	//Back face
	 1, -1, -1,
	-1, -1, -1,
	-1,  1, -1, 
	-1,  1, -1,
	 1,  1, -1, 
	 1, -1, -1,

	//Left face
	-1, -1, -1,
	-1, -1,  1,
	-1,  1,  1,
	-1,  1,  1,
	-1,  1, -1,
	-1, -1, -1,

	//Front face
	-1, -1,  1,
	 1, -1,  1,
	 1,  1,  1, 
	 1,  1,  1,
	-1,  1,  1, 
	-1, -1,  1,

	//Right face
	 1, -1,  1,
	 1, -1, -1,
	 1,  1, -1,
	 1,  1, -1,
	 1,  1,  1,
	 1, -1,  1,
	
	//Bottom face
	-1, -1, -1,
	 1, -1, -1,
	 1, -1,  1,
	 1, -1,  1,
	-1, -1,  1,
	-1, -1, -1,
	
	//Top face
	-1,  1, -1,
	 1,  1, -1,
	 1,  1,  1,
	 1,  1,  1,
	-1,  1,  1,
	-1,  1, -1,


	//Texcoords. It's a repeat of 6 texcoord pairs, for each face
	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0,

	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0,

	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0,

	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0,

	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0,

	0, 0,
	1, 0,
	1, 1,
	1, 1,
	0, 1,
	0, 0
};

vec3 eyeDirection = { 0.0, 0.0, 1.0 };
vec3 eyePosition  = { 0.0, 0.0, 0.0 };
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
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(36 * 3 * sizeof(float)));
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

	cubeTexture = loadTextureFarbfeld("textures/test.ff");
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
generateChunkModel(unsigned int x, unsigned int y, unsigned int z)
{
	if(worldChunkBuffer != 0)
		glDeleteBuffers(1, &worldChunkBuffer);

	worldChunkFaces = 0;

	glGenBuffers(1, &worldChunkBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, worldChunkBuffer);
	glBufferData(GL_ARRAY_BUFFER, WORLD_CHUNK_NBLOCKS * 30 * 6 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	float* bufferData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	float* vertexData = &bufferData[0];
	float* texcoordData = &bufferData[WORLD_CHUNK_NBLOCKS * 18 * 6];
	
	const WorldChunk* wChunk = getWorldChunk(x, y, z);
	for(int xb = 0; xb < WORLD_CHUNK_SIZE; xb++) 
		for(int yb = 0; yb < WORLD_CHUNK_SIZE; yb++) 
			for(int zb = 0; zb < WORLD_CHUNK_SIZE; zb++) {
				#define GET_BLOCK(X, Y, Z) wChunk->blocks[X][Y][Z]
				unsigned int currentBlock = GET_BLOCK(xb, yb, zb);
				if(currentBlock != 0) {
					if(xb == 0 || GET_BLOCK(xb - 1, yb, zb) == 0) {
						//Generate left face
						vertexData[0]  = -1 + xb * 2; vertexData[1]  = -1 + yb * 2; vertexData[2]  = -1 + zb * 2;
						vertexData[3]  = -1 + xb * 2; vertexData[4]  = -1 + yb * 2; vertexData[5]  =  1 + zb * 2;
						vertexData[6]  = -1 + xb * 2; vertexData[7]  =  1 + yb * 2; vertexData[8]  =  1 + zb * 2;
						vertexData[9]  = -1 + xb * 2; vertexData[10] =  1 + yb * 2; vertexData[11] =  1 + zb * 2;
						vertexData[12] = -1 + xb * 2; vertexData[13] =  1 + yb * 2; vertexData[14] = -1 + zb * 2;
						vertexData[15] = -1 + xb * 2; vertexData[16] = -1 + yb * 2; vertexData[17] = -1 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
                        texcoordData[2]  = 1; texcoordData[3]  = 0;
                        texcoordData[4]  = 1; texcoordData[5]  = 1;
                        texcoordData[6]  = 1; texcoordData[7]  = 1;
                        texcoordData[8]  = 0; texcoordData[9]  = 1;
                        texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}

					if(yb == 0 || GET_BLOCK(xb, yb - 1, zb) == 0) {
						//Generate bottom face
						vertexData[0]  = -1.0 + xb * 2; vertexData[1]  = -1.0 + yb * 2; vertexData[2]  = -1.0 + zb * 2;
						vertexData[3]  =  1.0 + xb * 2; vertexData[4]  = -1.0 + yb * 2; vertexData[5]  = -1.0 + zb * 2;
						vertexData[6]  =  1.0 + xb * 2; vertexData[7]  = -1.0 + yb * 2; vertexData[8]  =  1.0 + zb * 2;
						vertexData[9]  =  1.0 + xb * 2; vertexData[10] = -1.0 + yb * 2; vertexData[11] =  1.0 + zb * 2;
						vertexData[12] = -1.0 + xb * 2; vertexData[13] = -1.0 + yb * 2; vertexData[14] =  1.0 + zb * 2;
						vertexData[15] = -1.0 + xb * 2; vertexData[16] = -1.0 + yb * 2; vertexData[17] = -1.0 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
                        texcoordData[2]  = 1; texcoordData[3]  = 0;
                        texcoordData[4]  = 1; texcoordData[5]  = 1;
                        texcoordData[6]  = 1; texcoordData[7]  = 1;
                        texcoordData[8]  = 0; texcoordData[9]  = 1;
                        texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}
					if(xb == WORLD_CHUNK_SIZE - 1 || GET_BLOCK(xb + 1, yb, zb) == 0) {
						//Generate right face
						vertexData[0]  =  1 + xb * 2; vertexData[1]  = -1 + yb * 2; vertexData[2]  =  1 + zb * 2;
						vertexData[3]  =  1 + xb * 2; vertexData[4]  = -1 + yb * 2; vertexData[5]  = -1 + zb * 2;
						vertexData[6]  =  1 + xb * 2; vertexData[7]  =  1 + yb * 2; vertexData[8]  = -1 + zb * 2;
						vertexData[9]  =  1 + xb * 2; vertexData[10] =  1 + yb * 2; vertexData[11] = -1 + zb * 2;
						vertexData[12] =  1 + xb * 2; vertexData[13] =  1 + yb * 2; vertexData[14] =  1 + zb * 2;
						vertexData[15] =  1 + xb * 2; vertexData[16] = -1 + yb * 2; vertexData[17] =  1 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
                        texcoordData[2]  = 1; texcoordData[3]  = 0;
                        texcoordData[4]  = 1; texcoordData[5]  = 1;
                        texcoordData[6]  = 1; texcoordData[7]  = 1;
                        texcoordData[8]  = 0; texcoordData[9]  = 1;
                        texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}

					if(yb == WORLD_CHUNK_SIZE - 1 || GET_BLOCK(xb, yb + 1, zb) == 0) {
						//Generate top face
						vertexData[0]  = -1 + xb * 2; vertexData[1]  =  1 + yb * 2; vertexData[2]  = -1 + zb * 2;
						vertexData[3]  = -1 + xb * 2; vertexData[4]  =  1 + yb * 2; vertexData[5]  =  1 + zb * 2;
						vertexData[6]  =  1 + xb * 2; vertexData[7]  =  1 + yb * 2; vertexData[8]  =  1 + zb * 2;
						vertexData[9]  =  1 + xb * 2; vertexData[10] =  1 + yb * 2; vertexData[11] =  1 + zb * 2;
						vertexData[12] =  1 + xb * 2; vertexData[13] =  1 + yb * 2; vertexData[14] = -1 + zb * 2;
						vertexData[15] = -1 + xb * 2; vertexData[16] =  1 + yb * 2; vertexData[17] = -1 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
						texcoordData[2]  = 1; texcoordData[3]  = 0;
						texcoordData[4]  = 1; texcoordData[5]  = 1;
						texcoordData[6]  = 1; texcoordData[7]  = 1;
						texcoordData[8]  = 0; texcoordData[9]  = 1;
						texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}

					if(zb == WORLD_CHUNK_SIZE - 1 || !GET_BLOCK(xb, yb, zb + 1)) {
						//Generate front face
						vertexData[0]  = -1 + xb * 2; vertexData[1]  = -1 + yb * 2; vertexData[2]  =  1 + zb * 2;
						vertexData[3]  =  1 + xb * 2; vertexData[4]  = -1 + yb * 2; vertexData[5]  =  1 + zb * 2;
						vertexData[6]  =  1 + xb * 2; vertexData[7]  =  1 + yb * 2; vertexData[8]  =  1 + zb * 2;
						vertexData[9]  =  1 + xb * 2; vertexData[10] =  1 + yb * 2; vertexData[11] =  1 + zb * 2;
						vertexData[12] = -1 + xb * 2; vertexData[13] =  1 + yb * 2; vertexData[14] =  1 + zb * 2;
						vertexData[15] = -1 + xb * 2; vertexData[16] = -1 + yb * 2; vertexData[17] =  1 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
                        texcoordData[2]  = 1; texcoordData[3]  = 0;
                        texcoordData[4]  = 1; texcoordData[5]  = 1;
                        texcoordData[6]  = 1; texcoordData[7]  = 1;
                        texcoordData[8]  = 0; texcoordData[9]  = 1;
                        texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}

					if(zb == 0 || !GET_BLOCK(xb, yb, zb - 1)) {
						//Generate back face
						vertexData[0]  =  1 + xb * 2; vertexData[1]  = -1 + yb * 2; vertexData[2]  = -1 + zb * 2;
						vertexData[3]  = -1 + xb * 2; vertexData[4]  = -1 + yb * 2; vertexData[5]  = -1 + zb * 2;
						vertexData[6]  = -1 + xb * 2; vertexData[7]  =  1 + yb * 2; vertexData[8]  = -1 + zb * 2;
						vertexData[9]  = -1 + xb * 2; vertexData[10] =  1 + yb * 2; vertexData[11] = -1 + zb * 2;
						vertexData[12] =  1 + xb * 2; vertexData[13] =  1 + yb * 2; vertexData[14] = -1 + zb * 2;
						vertexData[15] =  1 + xb * 2; vertexData[16] = -1 + yb * 2; vertexData[17] = -1 + zb * 2;

						texcoordData[0]  = 0; texcoordData[1]  = 0;
                        texcoordData[2]  = 1; texcoordData[3]  = 0;
                        texcoordData[4]  = 1; texcoordData[5]  = 1;
                        texcoordData[6]  = 1; texcoordData[7]  = 1;
                        texcoordData[8]  = 0; texcoordData[9]  = 1;
                        texcoordData[10] = 0; texcoordData[11] = 0;

						vertexData += 18;
						texcoordData += 12;
						worldChunkFaces++;
					}

				}
			}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if(!worldChunkVao)
		glGenVertexArrays(1, &worldChunkVao);
	
	glBindVertexArray(worldChunkVao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, worldChunkBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(WORLD_CHUNK_NBLOCKS * 18 * 6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void
render()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glUseProgram(cubeRenderingShader);
	
	glUniformMatrix4fv(cubeUniformViewLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, cubeTexture);
//	glBindVertexArray(cubeVertexArray);
//	glDrawArrays(GL_TRIANGLES, 0, 36);
//	glBindVertexArray(0);
//	glBindTexture(GL_TEXTURE_2D, 0);

	if(worldChunkBuffer) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);	
		glBindVertexArray(worldChunkVao);
		glDrawArrays(GL_TRIANGLES, 0, worldChunkFaces * 6);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
}

void
setCamera(vec3 position, vec3 normal, vec3 direction)
{
	mat4x4_look_at(viewMatrix, position, direction, normal);
}
