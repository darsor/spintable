//Implementation file for the GPS class.
#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
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

float Gps::getTime() {
    float ftime;
    char type[6];
    type[5] = '\0';
    bool match = false;
    // loop until all data is read
    do {
        // jump to beginning of sentence ($)
        while ( ((char) serialGetchar(fd)) != '$');
        // make sure it's a GPGGA sentence
        for (int i=0; i<5; i++) {
            type[i]=serialGetchar(fd);
        }
        if (strcmp(type, "GPGGA") == 0) {
            match = true;
            // jump to ','
            while ( ((char) serialGetchar(fd)) != ',');
            // get time
            for (int i=0; i<10; i++) {
                buffer[i] = (char) serialGetchar(fd);
            }
        }
        while ( ((char) serialGetchar(fd)) != '\n');
    } while (serialDataAvail(fd) || !match);
    // get rid of the rest of the data
    //while (serialDataAvail(fd)) serialGetchar(fd);
    ftime = atof(buffer);
    //printf("GPS time: %d\n", ftime);
    return ftime;
}
