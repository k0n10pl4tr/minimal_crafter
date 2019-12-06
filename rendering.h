#include "linmath.h"

void initRenderingSystem(); 
void resizeRenderingSystem(int w, int h);

void generateChunkModel(unsigned int x, unsigned int y, unsigned int z);
void generateChunkModelAroundPos(int x, int y, int z, unsigned int distance);

void render(float camX, float camY, float camZ);

void setCamera(vec3 position, vec3 normal, vec3 direction);
