
typedef float TexcoordFace[12];

typedef enum BlockFaceDirection {
	BLOCK_TOP = 0,
	BLOCK_BOTTOM,
	BLOCK_LEFT,
	BLOCK_RIGHT,
	BLOCK_FRONT,
	BLOCK_BACK
} BlockFaceDirection;

typedef struct BlockInfo {
	const char *name;
	TexcoordFace texcoords[6];
} BlockInfo;

extern BlockInfo BLOCKS[3];

