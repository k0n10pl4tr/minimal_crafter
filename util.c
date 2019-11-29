#include "util.h"

#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct timespec clockStartSpec;

void
startClock()
{
	clock_gettime(CLOCK_REALTIME, &clockStartSpec);
	printf("Clock started with: %d.%llu\n", clockStartSpec.tv_sec, clockStartSpec.tv_nsec);
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
