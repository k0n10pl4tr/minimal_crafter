#include "blocks.h"

#define NUM_TEXTURES_ROW 16
#define TEXCOORD_DELTA (1.0 / NUM_TEXTURES_ROW)

#define GEN_TEXCOORD(TEX_X, TEX_Y) { \
	 (TEX_X + 0) * TEXCOORD_DELTA, (TEX_Y + 1) * TEXCOORD_DELTA, \
	 (TEX_X + 1) * TEXCOORD_DELTA, (TEX_Y + 1) * TEXCOORD_DELTA, \
	 (TEX_X + 1) * TEXCOORD_DELTA, (TEX_Y + 0) * TEXCOORD_DELTA, \
	 (TEX_X + 1) * TEXCOORD_DELTA, (TEX_Y + 0) * TEXCOORD_DELTA, \
	 (TEX_X + 0) * TEXCOORD_DELTA, (TEX_Y + 0) * TEXCOORD_DELTA, \
	 (TEX_X + 0) * TEXCOORD_DELTA, (TEX_Y + 1) * TEXCOORD_DELTA  \
}

#define GEN_SIMPLE_TEXCOORD(TEX_ID) GEN_TEXCOORD(TEX_ID % NUM_TEXTURES_ROW, TEX_ID / NUM_TEXTURES_ROW)
#define SIMPLE_TEXCOORD(TEX_ID) { GEN_SIMPLE_TEXCOORD(TEX_ID), GEN_SIMPLE_TEXCOORD(TEX_ID), GEN_SIMPLE_TEXCOORD(TEX_ID), GEN_SIMPLE_TEXCOORD(TEX_ID), GEN_SIMPLE_TEXCOORD(TEX_ID), GEN_SIMPLE_TEXCOORD(TEX_ID) } 

#define MULTI_TEXCOORD(TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK) { \
	GEN_SIMPLE_TEXCOORD(TOP), \
	GEN_SIMPLE_TEXCOORD(BOTTOM), \
	GEN_SIMPLE_TEXCOORD(LEFT), \
	GEN_SIMPLE_TEXCOORD(RIGHT), \
	GEN_SIMPLE_TEXCOORD(FRONT), \
	GEN_SIMPLE_TEXCOORD(BACK) \
}

BlockInfo BLOCKS[3] = {
	{ "GRASS", MULTI_TEXCOORD(0, 1, 2, 2, 2, 2) },
	{ "DIRT",  SIMPLE_TEXCOORD(1) },
	{ "STONE", SIMPLE_TEXCOORD(3) }
};
