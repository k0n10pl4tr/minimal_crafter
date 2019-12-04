#include "world.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

struct {
	unsigned int width, height, depth;
	unsigned long long int seed;
	WorldChunk* cachedChunks;
} world;

void
createWorld(unsigned int w, unsigned int h, unsigned int d)
{
	world.width  = w;
	world.height = h;
	world.depth  = d;
	world.seed    = getTimeNanoseconds();
	
	world.cachedChunks = malloc(sizeof(WorldChunk) * w * h * d);
	
	srand(world.seed);
	for(unsigned int i = 0; i < w * h * d; i++) {
		unsigned int x = i % w;
		unsigned int y = i / w;
		unsigned int z = i / (w * h);

		world.cachedChunks[i].xOffset = x;
		world.cachedChunks[i].yOffset = y;
		world.cachedChunks[i].zOffset = z;
		
		for(int xb = 0; xb < WORLD_CHUNK_SIZE; xb++)
			for(int yb = 0; yb < WORLD_CHUNK_SIZE; yb++)
				for(int zb = 0; zb < WORLD_CHUNK_SIZE; zb++) {
					world.cachedChunks[i].blocks[xb][yb][zb] = rand() % 4;
				}	
	}
}

const WorldChunk*
getWorldChunk(unsigned int xC, unsigned int yC, unsigned int zC)
{
	return &world.cachedChunks[xC + yC * world.width + zC * world.height * world.width];
}
