#include "aabb.h"
#include "world.h"

AABBCollisionBit
checkCollision(const AABBCollisor *a, const AABBCollisor *b)
{
	AABBCollisionBit xState = a->x + a->w >= b->x && b->x + b->w >= a->x;
	AABBCollisionBit yState = a->y + a->h >= b->y && b->y + b->h >= a->y;
	AABBCollisionBit zState = a->z + a->d >= b->z && b->z + b->d >= a->z;

	return xState && yState && zState;
}

AABBCollisionBit
checkCollisionWorld(const AABBCollisor* a)
{
	int xInt = (int)a->x;
	int yInt = (int)a->y;
	int zInt = (int)a->z;
	
	int wInt = (int)a->w;
	int hInt = (int)a->h;
	int dInt = (int)a->d;

	union {
		struct {
			unsigned int topLeftBackBlock;   
			unsigned int topRightBackBlock;
			unsigned int topLeftFrontBlock;    
			unsigned int topRightFrontBlock;   
			unsigned int bottomLeftBackBlock;  
			unsigned int bottomRightBackBlock; 
			unsigned int bottomLeftFrontBlock;
			unsigned int bottomRightFrontBlock;
		};
		unsigned int data[8];
	} blocks;

	blocks.topLeftBackBlock      = getWorldBlock(xInt, yInt + hInt, zInt);
	blocks.topRightBackBlock     = getWorldBlock(xInt + wInt, yInt + hInt, zInt);
	blocks.topLeftFrontBlock     = getWorldBlock(xInt, yInt + hInt, zInt + dInt);
	blocks.topRightFrontBlock    = getWorldBlock(xInt + wInt, yInt + hInt, zInt + dInt);
	blocks.bottomLeftBackBlock   = getWorldBlock(xInt, yInt, zInt);
	blocks.bottomRightBackBlock  = getWorldBlock(xInt + wInt, yInt, zInt);
	blocks.bottomLeftFrontBlock  = getWorldBlock(xInt, yInt, zInt + dInt);
	blocks.bottomRightFrontBlock = getWorldBlock(xInt + wInt, yInt, zInt + dInt);

	for(unsigned int i = 0; i < 8; i++) {
		if(blocks.data[i] != 0)
			return 1;
	}

	return 0;
}
