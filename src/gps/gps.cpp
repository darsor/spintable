//Implementation file for the GPS class.
#include "gps.h"
#include <wiringSerial.h>
#include <wiringPi.h>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <mutex>
#include <condition_variable>

void ppsISR();
std::mutex isr_mutex;
std::condition_variable cond_var;
uint64_t timestamp;
volatile bool pps;

Gps::Gps(int ppsPin, std::string device, int baud): ppsPin(ppsPin) {
    if ((fd = serialOpen(device.c_str(), baud)) < 0) {
        throw 1;
    }
    if (wiringPiSetup() < 0) {
        throw 1;
    }
    delay(5);
    serialPrintf(fd, "$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
    delay(5);

    pps = false;

    // mess with thread priority to make sure the ISR is launched with high priority
    // set high priority for this thread
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -3 -p " << pid;
    if (pid > 0)
        system(cmd.str().c_str());
    wiringPiISR(ppsPin, INT_EDGE_RISING, &ppsISR);
    // set low priority for this thread
    std::stringstream cmd2;
    cmd2 << "renice -n 0 -p " << pid;
    if (pid > 0)
        system(cmd2.str().c_str());
}

Gps::~Gps() {
    serialClose(fd);
}

void Gps::timestampPPS(uint64_t &systemTime) {
    pps = false;
    std::unique_lock<std::mutex> lk(isr_mutex);
    if (!cond_var.wait_for(lk, std::chrono::seconds(2) ,[]{return pps;})) {
        systemTime = 0;
        return;
    }
    pps = false;
    systemTime = timestamp;
}

float Gps::getTime() {
    float ftime;
    char type[6];
    char tmp;
    type[5] = '\0';
    bool match = false;
    // loop until all data is read
    do {
        // jump to beginning of sentence ($)
        do { 
            tmp = (char) serialGetchar(fd);
            if (tmp == 255) throw 1;
        } while (tmp != '$');
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

bool Gps::dataAvail() {
    return (serialDataAvail(fd) <= 0) ? false : true;
}

void ppsISR() {
    timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    {
        std::lock_guard<std::mutex> lk(isr_mutex);
        pps = true;
    }
    cond_var.notify_one();
}
