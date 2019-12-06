#include "rendering.h"

#include "glad.h"
#include "linmath.h"

#include "glutil.h"
#include "world.h"
#include "blocks.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//for memcpy
#include <string.h>

#define NUM_CHUNKS_RENDER 4
#define NUM_CHUNK_CACHED 256

typedef struct ChunkBufferData {
	unsigned int vao;
	unsigned int vbo;
	unsigned int faces;
	unsigned int xOffset, yOffset, zOffset;

	unsigned char canDraw;
} ChunkBufferData;

void generateFace(BlockFaceDirection face, unsigned int blockId, float *vertexData, float *texcoordData, int xb, int yb, int zb);
unsigned char isChunkInsideRenderBoundry(ChunkBufferData* bData, int pX, int pY, int pZ, int bDistance);

static unsigned int cubeRenderingShader = 0;
static unsigned int cubeVertexArray = 0;
static unsigned int cubeVertexBuffer = 0;

static unsigned int cubeUniformProjectionLocation = 0;
static unsigned int cubeUniformModelLocation      = 0;
static unsigned int cubeUniformViewLocation       = 0;

static unsigned int cubeTexture = 0;
static unsigned int cubeTerrainTexture = 0;

static unsigned int currentBlock = 0;

static const WorldChunk* wChunk;
static mat4x4 projectionMatrix;
static mat4x4 modelMatrix;
static mat4x4 viewMatrix;

static unsigned int chunkCachedStackSize = 0;
static ChunkBufferData chunkCached[NUM_CHUNK_CACHED];

#define TO_RADIANS(x) (x * M_PI) / 180.0

static const float cubeBufferData[] = {
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
	mat4x4_identity(viewMatrix);

	glUseProgram(cubeRenderingShader);
	glUniformMatrix4fv(cubeUniformProjectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(cubeUniformModelLocation,      1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(cubeUniformViewLocation,       1, GL_FALSE, &viewMatrix[0][0]);
	glUseProgram(0);
	
	cubeTexture        = loadTextureFarbfeld("textures/test.ff");
	cubeTerrainTexture = loadTextureFarbfeld("textures/terrain.ff");

	for(unsigned int i = 0; i < NUM_CHUNK_CACHED; i++) {
		chunkCached[i].canDraw = 0;
		glGenBuffers(1, &chunkCached[i].vbo);
		glGenVertexArrays(1, &chunkCached[i].vao);
			
		glBindBuffer(GL_ARRAY_BUFFER, chunkCached[i].vbo);
		glBufferData(GL_ARRAY_BUFFER, WORLD_CHUNK_NBLOCKS * 30 * 6 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(chunkCached[i].vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, chunkCached[i].vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)(WORLD_CHUNK_NBLOCKS * 18 * 6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void
resizeRenderingSystem(int w, int h)
{
	printf("Resizing to %dx%d (aspect: %f\n", w, h, (float)w/h);
	mat4x4_perspective(projectionMatrix, TO_RADIANS(70.0), (float)w/h, 0.01, 1000.0);
	glUseProgram(cubeRenderingShader);
	glUniformMatrix4fv(cubeUniformProjectionLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUseProgram(0);

	glViewport(0, 0, w, h);
}

void
generateChunkModel(unsigned int x, unsigned int y, unsigned int z)
{
	wChunk = getWorldChunk(x, y, z);
	if(wChunk->numBlocks == 0) {
		return;
	}
	unsigned int chunkId = chunkCachedStackSize % NUM_CHUNK_CACHED;
	for(unsigned int i = 0; i < NUM_CHUNK_CACHED; i++) {
		if(chunkCached[i].canDraw && 
				chunkCached[i].xOffset == x &&
				chunkCached[i].yOffset == y &&
				chunkCached[i].zOffset == z) {
			return;
		}
	}
	
	chunkCached[chunkId].faces = 0;
	chunkCached[chunkId].xOffset = x;
	chunkCached[chunkId].yOffset = y;
	chunkCached[chunkId].zOffset = z;
	
	glBindBuffer(GL_ARRAY_BUFFER, chunkCached[chunkId].vbo);
	float* bufferData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	float* vertexData = &bufferData[0];
	float* texcoordData = &bufferData[WORLD_CHUNK_NBLOCKS * 18 * 6];
	
	for(int i = 0; i < WORLD_CHUNK_NBLOCKS; i++) { 
		int xb = (i  % WORLD_CHUNK_SIZE);
		int yb = (i  / WORLD_CHUNK_SIZE) % WORLD_CHUNK_SIZE;
		int zb = (i / (WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE));

		int xp = xb + (int)x * WORLD_CHUNK_SIZE;
		int yp = yb + (int)y * WORLD_CHUNK_SIZE;
		int zp = zb + (int)z * WORLD_CHUNK_SIZE;
		
		#define GET_BLOCK(X, Y, Z) wChunk->blocks[X][Y][Z]
		currentBlock = GET_BLOCK(xb, yb, zb);
		if(currentBlock == 0)
			continue;
		if(xb == 0 || GET_BLOCK(xb - 1, yb, zb) == 0) {
			generateFace(BLOCK_LEFT, currentBlock, vertexData, texcoordData, xp, yp, zp);
			
			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}

		if(yb == 0 || GET_BLOCK(xb, yb - 1, zb) == 0) {
			generateFace(BLOCK_BOTTOM, currentBlock, vertexData, texcoordData, xp, yp, zp);

			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}
		if(xb == WORLD_CHUNK_SIZE - 1 || GET_BLOCK(xb + 1, yb, zb) == 0) {
			generateFace(BLOCK_RIGHT, currentBlock, vertexData, texcoordData, xp, yp, zp);

			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}

		if(yb == WORLD_CHUNK_SIZE - 1 || GET_BLOCK(xb, yb + 1, zb) == 0) {
			generateFace(BLOCK_TOP, currentBlock, vertexData, texcoordData, xp, yp, zp);

			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}

		if(zb == WORLD_CHUNK_SIZE - 1 || !GET_BLOCK(xb, yb, zb + 1)) {
			generateFace(BLOCK_FRONT, currentBlock, vertexData, texcoordData, xp, yp, zp);

			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}

		if(zb == 0 || !GET_BLOCK(xb, yb, zb - 1)) {
			generateFace(BLOCK_BACK, currentBlock, vertexData, texcoordData, xp, yp, zp);

			vertexData += 18;
			texcoordData += 12;
			chunkCached[chunkId].faces++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	chunkCached[chunkId].canDraw = 1;
	chunkCachedStackSize ++;
}

void 
generateChunkModelAroundPos(int x, int y, int z, unsigned int distance)
{
	int chunks = distance;
	x = x / WORLD_CHUNK_SIZE;
	y = y / WORLD_CHUNK_SIZE;
	z = z / WORLD_CHUNK_SIZE;

	for(int xc = x - chunks; xc < x + chunks; xc++) {
		for(int yc = y - chunks; yc < y + chunks; yc++) {
			for(int zc = z - chunks; zc < z + chunks; zc++) {
				const WorldChunk* chunk = getWorldChunk(xc, yc, zc);
				if(chunk == NULL) 
					continue;
				generateChunkModel(xc, yc, zc);
			}
		}
	}
}

void
render(float camX, float camY, float camZ)
{
	generateChunkModelAroundPos(camX, camY, camZ, NUM_CHUNKS_RENDER);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glUseProgram(cubeRenderingShader);
	glClearColor(0.2, 0.3, 0.7, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUniformMatrix4fv(cubeUniformViewLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	for(unsigned int i = 0; i < NUM_CHUNK_CACHED; i++) {
		if(chunkCached[i].canDraw && isChunkInsideRenderBoundry(&chunkCached[i], camX, camY, camZ, 4)) {
			glUniformMatrix4fv(cubeUniformModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cubeTerrainTexture);	
			glBindVertexArray(chunkCached[i].vao);
			glDrawArrays(GL_TRIANGLES, 0, chunkCached[i].faces * 6);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
}

void
setCamera(vec3 position, vec3 normal, vec3 direction)
{
	mat4x4_look_at(viewMatrix, position, direction, normal);
}

void
generateFace(BlockFaceDirection face, unsigned int blockId, float *vertexData, float *texcoordData, int xb, int yb, int zb)
{
	switch(face) {
	case BLOCK_LEFT:
	vertexData[0]  =  0 + xb; vertexData[1]  =  0 + yb; vertexData[2]  =  0 + zb;
	vertexData[3]  =  0 + xb; vertexData[4]  =  0 + yb; vertexData[5]  =  1 + zb;
	vertexData[6]  =  0 + xb; vertexData[7]  =  1 + yb; vertexData[8]  =  1 + zb;
	vertexData[9]  =  0 + xb; vertexData[10] =  1 + yb; vertexData[11] =  1 + zb;
	vertexData[12] =  0 + xb; vertexData[13] =  1 + yb; vertexData[14] =  0 + zb;
	vertexData[15] =  0 + xb; vertexData[16] =  0 + yb; vertexData[17] =  0 + zb;
	break;
	case BLOCK_BOTTOM:
	vertexData[0]  =  0 + xb; vertexData[1]  =  0 + yb; vertexData[2]  =  0 + zb;
	vertexData[3]  =  1 + xb; vertexData[4]  =  0 + yb; vertexData[5]  =  0 + zb;
	vertexData[6]  =  1 + xb; vertexData[7]  =  0 + yb; vertexData[8]  =  1 + zb;
	vertexData[9]  =  1 + xb; vertexData[10] =  0 + yb; vertexData[11] =  1 + zb;
	vertexData[12] =  0 + xb; vertexData[13] =  0 + yb; vertexData[14] =  1 + zb;
	vertexData[15] =  0 + xb; vertexData[16] =  0 + yb; vertexData[17] =  0 + zb;
	break;
	case BLOCK_RIGHT:
	vertexData[0]  =  1 + xb; vertexData[1]  =  0 + yb; vertexData[2]  =  1 + zb;
	vertexData[3]  =  1 + xb; vertexData[4]  =  0 + yb; vertexData[5]  =  0 + zb;
	vertexData[6]  =  1 + xb; vertexData[7]  =  1 + yb; vertexData[8]  =  0 + zb;
	vertexData[9]  =  1 + xb; vertexData[10] =  1 + yb; vertexData[11] =  0 + zb;
	vertexData[12] =  1 + xb; vertexData[13] =  1 + yb; vertexData[14] =  1 + zb;
	vertexData[15] =  1 + xb; vertexData[16] =  0 + yb; vertexData[17] =  1 + zb;
	break;
	case BLOCK_TOP:
	vertexData[0]  =  0 + xb; vertexData[1]  =  1 + yb; vertexData[2]  =  0 + zb;
	vertexData[3]  =  0 + xb; vertexData[4]  =  1 + yb; vertexData[5]  =  1 + zb;
	vertexData[6]  =  1 + xb; vertexData[7]  =  1 + yb; vertexData[8]  =  1 + zb;
	vertexData[9]  =  1 + xb; vertexData[10] =  1 + yb; vertexData[11] =  1 + zb;
	vertexData[12] =  1 + xb; vertexData[13] =  1 + yb; vertexData[14] =  0 + zb;
	vertexData[15] =  0 + xb; vertexData[16] =  1 + yb; vertexData[17] =  0 + zb;
	break;	
	case BLOCK_FRONT:
	vertexData[0]  =  0 + xb; vertexData[1]  =  0 + yb; vertexData[2]  =  1 + zb;
	vertexData[3]  =  1 + xb; vertexData[4]  =  0 + yb; vertexData[5]  =  1 + zb;
	vertexData[6]  =  1 + xb; vertexData[7]  =  1 + yb; vertexData[8]  =  1 + zb;
	vertexData[9]  =  1 + xb; vertexData[10] =  1 + yb; vertexData[11] =  1 + zb;
	vertexData[12] =  0 + xb; vertexData[13] =  1 + yb; vertexData[14] =  1 + zb;
	vertexData[15] =  0 + xb; vertexData[16] =  0 + yb; vertexData[17] =  1 + zb;
	break;
	case BLOCK_BACK:
	vertexData[0]  =  1 + xb; vertexData[1]  =  0 + yb; vertexData[2]  =  0 + zb;
	vertexData[3]  =  0 + xb; vertexData[4]  =  0 + yb; vertexData[5]  =  0 + zb;
	vertexData[6]  =  0 + xb; vertexData[7]  =  1 + yb; vertexData[8]  =  0 + zb;
	vertexData[9]  =  0 + xb; vertexData[10] =  1 + yb; vertexData[11] =  0 + zb;
	vertexData[12] =  1 + xb; vertexData[13] =  1 + yb; vertexData[14] =  0 + zb;
	vertexData[15] =  1 + xb; vertexData[16] =  0 + yb; vertexData[17] =  0 + zb;
	break;
	}
	memcpy(texcoordData, BLOCKS[blockId - 1].texcoords[face], sizeof(TexcoordFace));
}

unsigned char 
isChunkInsideRenderBoundry(ChunkBufferData* bData, int x, int y, int z, int bDistance)
{
	x = x / WORLD_CHUNK_SIZE;
	y = y / WORLD_CHUNK_SIZE;
	z = z / WORLD_CHUNK_SIZE;

	int xd = abs((int)bData->xOffset - x);
	int yd = abs((int)bData->yOffset - y);
	int zd = abs((int)bData->zOffset - z);
	
	return xd < bDistance && yd < bDistance && zd < bDistance;
}
