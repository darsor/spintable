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
#include <cmath>

// gather data from various sensors
void gps(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);
void encoder(encoderPacket &p);

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[18]);
void convertEncoderData(encoderPacket &p, char buffer[30]);

// send packets
int sendTimePacket(timePacket &p, Cosmos &cosmos);
int sendEncoderPacket(encoderPacket &p, Cosmos &cosmos);

// initialize COSMOS and devices
// these are global for now so that the motor can access them
Cosmos cosmos(8321);
DCMotor motor(2, 0x60, 1600);
                                    // TODO: mutex motor functions

PI_THREAD (motorControl) {
    static char buffer[6];
    static uint16_t length;
    static uint16_t id;
    static int16_t speed;
    static float pos;
    static double position;
    while (true) {
//      while (cosmos.recvPacket(motorBuffer, 4) != 0) { // keep trying until the receive works
//          usleep(10000); // don't overload processor if this starts looping
//      }
        // receive the first two bytes (the length of the packet);
        if (cosmos.recvPacket(buffer, 2) != 0) {
            // if connection was lost, sending packets will fail and then try to reconnect
            motor.setSpeed(0);
            usleep(5000000);
        } else {
            length = ntohs(*((uint16_t*)buffer));
            printf("length of packet received: %d\n", length);
            // receive the rest of the packet
            cosmos.recvPacket(buffer, length);
            // get the id to know what command it is
            id = ntohs(*((uint16_t*)buffer));
        }
        switch (id) {
            case 1:
                motor.setHome();
                break;
            case 2:
                speed = ntohs(*((int16_t*)(buffer+2)));
                printf("received command to change speed to %d\n", speed);
                motor.setGradSpeed(speed);
                break;
            case 3:
                pos = *( (float*) (buffer+2));
                endianSwap(pos);
                printf("received command to change absolute position to %f\n", pos);
                motor.pidPosition(pos);
                break;
            case 4:
                pos = *( (float*) (buffer+2));
                endianSwap(pos);
                printf("received command to change relative position by %f\n", pos);
                position = motor.getPosition();
                position += pos;
                if (position < 360) position += 360;
                if (position > 360) position -= 360;
                motor.pidPosition(position);
                break;
            case 5:
                motor.gotoIndex();
                break;
            default:
                printf("unknown command received\n");
        }
        id = 0;
        usleep(50000);
    }
}

int main() {

    // set up wiringPi
    wiringPiSetup();

    // initialize packets
    struct timePacket tPacket;
    struct encoderPacket ePacket;

    // establish connection with COSMOS
    cosmos.acceptConnection();

    // initialize devices
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
    }

    while (true) {

        // get timestamps and send time packet
        gps(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        if (sendTimePacket(tPacket, cosmos) != 0) {
            printf("lost connection to COSMOS\n");
            motor.stopPID();
            cosmos.acceptConnection();
            printf("reconnected to COSMOS\n");
        }

        // every second, do this 50 times
        for (int j=0; j<50; j++) {
            systemTimestamp(ePacket.sysTimeSeconds, ePacket.sysTimeuSeconds);
            encoder(ePacket);
            sendEncoderPacket(ePacket, cosmos);
            usleep(17500); // TODO: fine tune the delay
        }
    }
    return 0;
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

void encoder(encoderPacket &p) {
    static const unsigned int CNT_PER_REV = 2400;
    p.motorSpeed = motor.getSpeed();
    p.position = motor.getPosition();
    p.raw_cnt = motor.getCnt();
    p.rev_cnt = p.raw_cnt / CNT_PER_REV;
}

void convertTimeData(timePacket &p, char buffer[18]) {
    static uint16_t u16;
    static uint32_t u32;
    static float f;
    u32 = htonl(p.length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4,  &u16, 2);
    f = p.gpsTime;
    endianSwap(f);
    memcpy(buffer+6,  &f, 4);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+10,  &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+14, &u32, 4);
}

void convertEncoderData(encoderPacket &p, char buffer[30]) {
    static uint16_t u16;
    static uint32_t u32;
    static int32_t i32;
    static float f;
    u32 = htonl(p.length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4,  &u16, 2);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+6,  &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+10, &u32, 4);
    i32 = htonl(p.raw_cnt);
    memcpy(buffer+14, &i32, 4);
    f = p.motorSpeed;
    endianSwap(f);
    memcpy(buffer+18, &f, 4);
    f = p.position;
    endianSwap(f);
    memcpy(buffer+22, &f, 4);
    u32 = htonl(p.rev_cnt);
    memcpy(buffer+26, &u32, 4);
}

int sendTimePacket(timePacket &p, Cosmos &cosmos) {
static char timeBuffer[18];
    convertTimeData(p, timeBuffer);
    return cosmos.sendPacket(timeBuffer, sizeof(timeBuffer));
}

int sendEncoderPacket(encoderPacket &p, Cosmos &cosmos) {
    static char encoderBuffer[30];
    convertEncoderData(p, encoderBuffer);
    return cosmos.sendPacket(encoderBuffer, sizeof(encoderBuffer));
}
