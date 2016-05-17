//Implementation file for the GPS class.
#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <cmath>
#include <string>
#include <sys/time.h>
#include <mutex>
#include <condition_variable>

void ppsISR();
std::mutex isr_mutex;
std::condition_variable cond_var;
timeval time_val;
volatile bool pps;

Gps::Gps(): Gps("/dev/ttyAMA0", 9600) {
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
    serialPrintf(fd, "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
    delay(5);

    pps = false;

    // mess with thread priority to make sure the ISR is launched with high priority
    // set high priority for this thread
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0)
        system(cmd.str().c_str());
    wiringPiISR(1, INT_EDGE_RISING, &ppsISR);
    // set low priority for this thread
    std::stringstream cmd2;
    cmd2 << "renice -n 0 -p " << pid;
    if (pid > 0)
        system(cmd2.str().c_str());
}

Gps::~Gps() {
    serialClose(fd);
}

void Gps::timestampPPS(uint32_t &stime, uint32_t &ustime) {
    std::unique_lock<std::mutex> lk(isr_mutex);
    cond_var.wait(lk, []{return pps;});
    pps = false;
    lk.unlock();
    stime = time_val.tv_sec;
    ustime = time_val.tv_usec;
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
    ftime = atof(buffer);
    //printf("GPS time: %f\n", ftime);
    return ftime;
}

void ppsISR() {
    gettimeofday(&time_val, NULL);
    {
        std::lock_guard<std::mutex> lk(isr_mutex);
        pps = true;
    }
    cond_var.notify_one();
}
