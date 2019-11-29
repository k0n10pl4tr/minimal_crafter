#include "util.h"

#define _POSIX_C_SOURCE 200809L
#include <time.h>

struct timespec clockStartSpec;

void
startClock()
{
	clock_gettime(CLOCK_REALTIME, &clockStartSpec);
}

unsigned long long
getCurrentTimeNano()
{
	struct timespec clockNow;
	clock_gettime(CLOCK_REALTIME, &clockNow);
	return (clockNow.tv_sec - clockStartSpec.tv_sec) * 1000000000 + clockNow.tv_nsec - clockStartSpec.tv_nsec;
}

