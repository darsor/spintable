#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <string>

Gps::Gps() {
    if ((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0) {
        perror("ERROR opening GPS");
        exit(1);
    }
    if (wiringPiSetup() < 0) {
        perror("ERROR setting up wiringPi");
        exit(1);
    }
    delay(5);
}

Gps::Gps(std::string device, int baud) {
    if ((fd = serialOpen(device.c_str(), baud)) < 0) {
        perror("ERROR opening GPS");
        exit(1);
    }
    if (wiringPiSetup() < 0) {
        perror("ERROR setting up wiringPi");
        exit(1);
    }
    delay(5);
}

Gps::~Gps() {
    serialClose(fd);
}

float Gps::getTime() {
    float ftime;
    while ( ((char) serialGetchar(fd)) != '$');
    while ( ((char) serialGetchar(fd)) != ',');
    for (int i=0; i<10; i++) {
        buffer[i] = (char) serialGetchar(fd);
        ftime = atof(buffer);
    }
    if (serialDataAvail(fd) > 70) {
        printf("WARNING: GPS stream backlogged\n");
    } else {
        return ftime;
    }
}
