#include "fvm400.h"
#include <unistd.h> // for usleep()
#include <cstdio> // for printf()
#include <sys/time.h> // for gettimeofday()

int main() {
    FVM400 mag("/dev/ttyUSB1");
    usleep(1000000);
    struct timeval start, next;
    long int difference;
    struct FVM400_data data;
    gettimeofday(&start, nullptr);
    for (int i=0; i<50; i++) {
        data = mag.getData();
    }
    gettimeofday(&next, nullptr);
    difference = next.tv_usec - start.tv_usec + (next.tv_sec - start.tv_sec) * 1000000;
    printf("FVM400 took %li microseconds to gather 50 samples\n", difference);

    return 0;
}
