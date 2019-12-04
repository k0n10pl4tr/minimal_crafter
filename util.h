
void startClock();
double getCurrentTimeNano();
void sleepNanosec(double time);

unsigned short bigEndianToHost16(unsigned short beValue);
unsigned int   bigEndianToHost32(unsigned int   beValue);

unsigned long long int getTimeNanoseconds();

unsigned short *getFarbfeldImageData(const char *path, unsigned int *w, unsigned int *h);
