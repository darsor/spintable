#include "gps.h"
#include <unistd.h>
#include <cstdio>
#include <atomic>
#include <sys/time.h>
#include <wiringPi.h>

timeval tv;
std::atomic<bool> pulse;

void ppsISR() {
    gettimeofday(&tv, NULL);
    printf("pps: ");
    pulse.store(true);
}

int main() {
    wiringPiSetup();
    pulse = false;
    Gps gps;
    wiringPiISR(1, INT_EDGE_RISING, &ppsISR);

    while(true) {
        //printf("gpstime: %f\n", gps.getTime());
        while (!pulse.load()) delay(1000);
        pulse.store(false);
        printf("gpstime: %f\n", gps.getTime());
        printf("systime: %li\n", tv.tv_usec);
    }
    return 0;
}
