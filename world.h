
#define WORLD_CHUNK_SIZE 16
#define WORLD_CHUNK_NBLOCKS (WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE * WORLD_CHUNK_SIZE)
#define WORLD_CHUNK_DATA_OFFSET sizeof(WorldChunk)

typedef struct WorldChunk {
	unsigned int xOffset, yOffset, zOffset;
	//                                 X                 Y                 Z
	unsigned int blocks[WORLD_CHUNK_SIZE][WORLD_CHUNK_SIZE][WORLD_CHUNK_SIZE];
} WorldChunk;


void createWorld(unsigned int w, unsigned int h, unsigned int d);
const WorldChunk* getWorldChunk(int xC, int yC, int zC);
