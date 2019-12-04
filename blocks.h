
typedef float TexcoordFace[12];

enum BlockFaceDirection {
	BLOCK_TOP = 0,
	BLOCK_BOTTOM,
	BLOCK_LEFT,
	BLOCK_RIGHT,
	BLOCK_FRONT,
	BLOCK_BACK
};
struct BlockInfo {
	const char *name;
	TexcoordFace texcoords[6];
};


extern struct BlockInfo BLOCKS[3];

