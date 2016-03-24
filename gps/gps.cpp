#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <termios.h>
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
    if (pinMode == NULL) {
        if (wiringPiSetup() < 0) {
            perror("ERROR setting up wiringPi");
            exit(1);
        }
    }
    delay(5);
    serialPrintf(fd, "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
    delay(5);
}

Gps::Gps(std::string device, int baud) {
    if ((fd = serialOpen(device.c_str(), baud)) < 0) {
        perror("ERROR opening GPS");
        exit(1);
    }
    if (pinMode == NULL) {
        if (wiringPiSetup() < 0) {
            perror("ERROR setting up wiringPi");
            exit(1);
        }
    }
    delay(5);
    serialPrintf(fd, "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
    delay(5);
}

Gps::~Gps() {
    serialClose(fd);
}

uint32_t Gps::getTime() {
    uint32_t ftime;
    // loop until all data is read
    do {
        // jump to beginning of sentence ($)
        while ( ((char) serialGetchar(fd)) != '$');
        // jump to ','
        while ( ((char) serialGetchar(fd)) != ',');
        // get time
        for (int i=0; i<10; i++) {
            buffer[i] = (char) serialGetchar(fd);
        }
        // read until end of sentence (\x0a)
        while ( ((char) serialGetchar(fd)) != '\n');
    } while (serialDataAvail(fd));
    ftime = atoi(buffer);
    //printf("GPS time: %d\n", ftime);
    return ftime;
}
