#include "util.h"

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

struct timespec clockStartSpec;

void
startClock()
{
	clock_gettime(CLOCK_REALTIME, &clockStartSpec);
	printf("Clock started with: %lu.%lu\n", clockStartSpec.tv_sec, clockStartSpec.tv_nsec);
}

double
getCurrentTimeNano()
{
	struct timespec clockNow;
	clock_gettime(CLOCK_REALTIME, &clockNow);
	return (clockNow.tv_sec - clockStartSpec.tv_sec) + (clockNow.tv_nsec - clockStartSpec.tv_nsec) / 1000000000.0;
}

void
sleepNanosec(double time)
{
	struct timespec sleepTime;
	struct timespec sleptTime;

	sleepTime.tv_sec  = (unsigned long long)floor(time);
	sleepTime.tv_nsec = (time - sleepTime.tv_sec) * 1000000000.0;

	nanosleep(&sleepTime, &sleptTime);
}

unsigned long long int
getTimeNanoseconds()
{
	struct timespec clockNow;
	clock_gettime(CLOCK_REALTIME, &clockNow);
	return (clockNow.tv_sec - clockStartSpec.tv_sec) * 1000000000.0 + (clockNow.tv_nsec - clockStartSpec.tv_nsec); 
}

unsigned short 
bigEndianToHost16(unsigned short beValue)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return beValue >> 8 | beValue << 8;
#else
	return beValue;
#endif
}

unsigned int
bigEndianToHost32(unsigned int beValue)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return 
		((beValue >> 24) & 0xFF      ) |
		((beValue <<  8) & 0xFF0000  ) |
		((beValue >>  8) & 0xFF00    ) |
		((beValue << 24) & 0xFF000000);
#else
	return beValue;
#endif
}

unsigned short *
getFarbfeldImageData(const char *path, unsigned int *wi, unsigned int *hi)
{
	char header[9];
	unsigned int w, h;
	unsigned short *imageData;
	
	FILE *fp = fopen(path, "r");
	if(!fp) {
		printf("Could not open the image: %s\n", path);
		return (unsigned short*)-1;
	}

	fread(header, 8, 1, fp);

	fread(&w, sizeof(w), 1, fp);
	fread(&h, sizeof(h), 1, fp);

	w = bigEndianToHost32(w);
	h = bigEndianToHost32(h);

	imageData = malloc(4 * w * h * sizeof(short));
	fread(imageData, 1, sizeof(short) * w * h * 4, fp);
	
	for(unsigned int i = 0; i < w * h * 4; i++) {
		imageData[i] = bigEndianToHost16(imageData[i]);
	}
	*wi = w;
	*hi = h;
	fclose(fp);
	return imageData;
}
