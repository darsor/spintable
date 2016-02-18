#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cmath>

Gps::Gps() {
    if ((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0) {
        perror("The following error occured");
    }
    if (wiringPiSetup() < 0) {
        perror("The following error occured");
    }
    delay(5);
}

Gps::Gps(char* device, int baud) {
    if ((fd = serialOpen(device, baud)) < 0) {
        perror("The following error occured");
    }
    if (wiringPiSetup() < 0) {
        perror("The following error occured");
    }
    delay(5);
}

Gps::~Gps() {
    serialClose(fd);
}

gpsTime Gps::getTime() {
    gpsTime time;
    float ftime;
    while ( ((char) serialGetchar(fd)) != '$');
    while ( ((char) serialGetchar(fd)) != ',');
    for (int i=0; i<10; i++) {
        buffer[i] = (char) serialGetchar(fd);
        ftime = atof(buffer);
        time.hhmmss = (uint16_t) ftime;
        time.sss = (uint16_t) 1000*(ftime - time.hhmmss);
    }
    if (serialDataAvail(fd) > 70) {
        printf("WARNING: GPS stream backlogged\n");
    } else {
        return time;
    }
}
