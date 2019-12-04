#include "linmath.h"

void initRenderingSystem(); 
void resizeRenderingSystem(int w, int h);

void generateChunkModel(unsigned int x, unsigned int y, unsigned int z);

void render();

void setCamera(vec3 position, vec3 normal, vec3 direction);
