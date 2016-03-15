#include "cosmos/cosmos.h"
#include "gps/gps.h"
#include "motor/dcmotor.h"
#include <wiringPi.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>

// report errors
void error(const char *msg);

// gather data from various sensors
void gps(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);
// TODO: get encoder data

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[18]);
// TODO: convert encoder data

// send packets
void sendTimePacket(timePacket &p, Cosmos &cosmos);
// TODO: send encoder packet

// control motor
void setSpeed(DCMotor &motor, motorPacket &p);

// initialize COSMOS
// this is global for now so that the motor can access it
Cosmos cosmos(8321);

PI_THREAD (motorControl) {
    static char motorBuffer[4];
    static motorPacket mPacket;
    static DCMotor motor(2, 0x60, 1600);
    while (true) {
        if (cosmos.recvPacket(motorBuffer, 4) == 0) {
            mPacket.speed = ntohs(*(motorBuffer));
            mPacket.id = ntohs(*(motorBuffer+2));
            if (mPacket.id = 1) {
                motor.setGradSpeed(mPacket.speed);
            }
        }
        usleep(20000);
    }
}

int main() {

    // set up wiringPi
    wiringPiSetup();
    std::system("gpio edge 1 rising");

    // initialize packets
    struct timePacket tPacket;
    struct encoderPacket ePacket;

    // establish connection with COSMOS
    cosmos.cosmosConnect();

    // initialize devices
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
    }

    while (true) {

        // get timestamps and send time packet
        waitForInterrupt (1, 2000);
        gps(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        sendTimePacket(tPacket, cosmos);

        // every second, do this 50 times
        for (int j=0; j<50; j++) {
//          systemTimestamp(ePacket.sysTimeSeconds, ePacket.sysTimeuSeconds);
            // TODO: get encoder data and send encoder packet
            systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
            sendTimePacket(tPacket, cosmos);
            usleep(10000); // TODO: fine tune the delay
        }
    }
    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void gps(timePacket &p) {
    static Gps gps("/dev/ttyAMA0", 9600);
    p.gpsTime = gps.getTime();
}

void systemTimestamp(uint32_t &stime, uint32_t &ustime) {
    static struct timeval timeVal;
    static struct timezone timeZone;
    gettimeofday(&timeVal, &timeZone);
    stime =  timeVal.tv_sec;
    ustime = timeVal.tv_usec;
}

void convertTimeData(timePacket &p, char buffer[18]) {
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(p.length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4,  &u16, 2);
    u32 = htonl(p.gpsTime);
    memcpy(buffer+6,  &u32, 4);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+10,  &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+14, &u32, 4);
}

void sendTimePacket(timePacket &p, Cosmos &cosmos) {
    static char timeBuffer[18];
    convertTimeData(p, timeBuffer);
    cosmos.sendPacket(timeBuffer, sizeof(timeBuffer));
}

void setSpeed(DCMotor &motor, motorPacket &p) {
    static int speed = 0;
    if (speed != p.speed) {
        speed = p.speed;
        motor.setGradSpeed(speed);
    }
}
