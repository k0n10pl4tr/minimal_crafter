#include "world.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

struct {
	unsigned int width, height, depth;
	unsigned long long int seed;
	WorldChunk* cachedChunks;
} world;

void generateChunk(WorldChunk *chunk, unsigned int xc, unsigned int yc, unsigned int zc, unsigned int seed);

void
createWorld(unsigned int w, unsigned int h, unsigned int d)
{
	world.width  = w;
	world.height = h;
	world.depth  = d;
	world.seed    = getTimeNanoseconds();
	
	world.cachedChunks = malloc(sizeof(WorldChunk) * w * h * d);
	
	for(unsigned int i = 0; i < w * h * d; i++) {
		unsigned int x = i % w;
		unsigned int y = (i / w) % h;
		unsigned int z = i / (w * h);

		world.cachedChunks[i].xOffset = x;
		world.cachedChunks[i].yOffset = y;
		world.cachedChunks[i].zOffset = z;
		
		generateChunk(&world.cachedChunks[i], x, y, z, world.seed);
	}
}

const WorldChunk*
getWorldChunk(int xC, int yC, int zC)
{
	if(xC < 0 || xC >= world.width || yC < 0 || yC >= world.height || zC < 0 || zC >= world.depth) {
		return NULL;
	}
	return &world.cachedChunks[xC + yC * world.width + zC * world.height * world.width];
}

unsigned int
getWorldBlock(int xB, int yB, int zB)
{
	const WorldChunk* chunk = getWorldChunk(xB / WORLD_CHUNK_SIZE, yB / WORLD_CHUNK_SIZE, zB / WORLD_CHUNK_SIZE);
	if(chunk == NULL)
		return 0;
	return chunk->blocks[xB % WORLD_CHUNK_SIZE][yB % WORLD_CHUNK_SIZE][zB % WORLD_CHUNK_SIZE];
}

void
generateChunk(WorldChunk *chunk, unsigned int xc, unsigned int yc, unsigned int zc, unsigned int seed)
{
	for(int i = 0; i < WORLD_CHUNK_NBLOCKS; i++) {
		int xb = (i % WORLD_CHUNK_SIZE);
		int yb = (i / WORLD_CHUNK_SIZE) % WORLD_CHUNK_SIZE;
		int zb = (i / (WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE));

		float height = stb_perlin_noise3_seed((float)(xb + xc * WORLD_CHUNK_SIZE) / WORLD_CHUNK_SIZE, 
				0, 
				(float)(zb + zc * WORLD_CHUNK_SIZE) / WORLD_CHUNK_SIZE,
				0, 0, 0, seed) * 4.0 + 16;

		int heightInt = floor(height * height) / 16;

		if(yb + yc * WORLD_CHUNK_SIZE == heightInt) 
			chunk->blocks[xb][yb][zb] = 1;
		else if(yb + yc * WORLD_CHUNK_SIZE > heightInt - 4 && yb + yc * WORLD_CHUNK_SIZE < heightInt)
			chunk->blocks[xb][yb][zb] = 2;
		else if(yb + yc * WORLD_CHUNK_SIZE <= heightInt - 4)
			chunk->blocks[xb][yb][zb] = 3;
		else
			chunk->blocks[xb][yb][zb] = 0;

		if (yb + yc * WORLD_CHUNK_SIZE <= heightInt) {
			chunk->numBlocks++;
		}
	}
}
