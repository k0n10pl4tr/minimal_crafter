
typedef struct AABBCollisor {
	float x, y, z;
	float w, h, d;
} AABBCollisor;

typedef unsigned char AABBCollisionBit;

AABBCollisionBit checkCollision(const AABBCollisor *a, const AABBCollisor *b);
AABBCollisionBit checkCollisionWorld(const AABBCollisor* a);
