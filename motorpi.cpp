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
void gps(TimePacket* p);
void encoder(EncoderPacket* p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// send packets
int sendTimePacket(timePacket &p, Cosmos &cosmos);
int sendEncoderPacket(encoderPacket &p, Cosmos &cosmos);

// initialize COSMOS and devices
// these are global so that all threads can access them
CosmosQueue<Packet*> queue(128);
DCMotor motor(2, 0x60, 1600);

PI_THREAD (motorControl) {
    static char buffer[6];
    static uint16_t length;
    static uint16_t id;
    static int16_t speed;
    static float pos;
    static double position;
    while (true) {
        // receive the first two bytes (the length of the packet);
        if (cosmos.recvPacket(buffer, 2) != 0) {
            // if connection was lost, sending packets will fail and then try to reconnect
            motor.stopPID();
            motor.setSpeed(0);
        } else {
            length = ntohs(*((uint16_t*)buffer));
            // receive the rest of the packet
            cosmos.recvPacket(buffer, length);
            // get the id to know what command it is
            id = ntohs(*((uint16_t*)buffer));
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
                    printf("received command to gotoIndex\n");
                    motor.gotoIndex();
                    break;
                default:
                    printf("unknown command received\n");
            }
        }
        id = 0;
        while (!queue.isConnected()) usleep(50000); // if it's disconnected, loop and wait until it reconnects
    }
}

PI_THREAD (cosmosQueue) {
    while (true) {
        queue.connect();
        while (queue.isConnected()) {
            while (queue.pop());
            usleep(10000);
        }
        printf("connection with COSMOS lost\n");
    }
}

int main() {

    // set up wiringPi
    wiringPiSetup();
    piHiPri(99);

    // initialize packets
    TimePacket* tPacket = NULL;
    EncoderPacket* ePacket = NULL;

    // initialize devices
    gps(tPacket);
    encoder(ePacket);

    // start threads
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
    }
    if (piThreadCreate(cosmosQueue) != 0) {
        perror("COSMOS queue thread didn't start");
    }

    while (true) {

        // get timestamps and send time packet
        tPacket = new TimePacket();
        gps(tPacket);
        systemTimestamp(tPacket->sysTimeSeconds, tPacket->sysTimeuSeconds);
        queue.push(tPacket);

        // TODO: test this entire program =D
        // every second, do this 50 times
        for (int j=0; j<50; j++) {
            ePacket = new EncoderPacket();
            systemTimestamp(ePacket.sysTimeSeconds, ePacket.sysTimeuSeconds);
            encoder(ePacket);
            queue.push(tPacket);
            usleep(17500); // TODO: fine tune the delay
        }
    }
    return 0;
}

void gps(TimePacket* p) {
    static Gps gps("/dev/ttyAMA0", 9600);
    if (p == NULL) return;
    p->gpsTime = gps.getTime();
}

void systemTimestamp(uint32_t &stime, uint32_t &ustime) {
    static struct timeval timeVal;
    gettimeofday(&timeVal, NULL);
    stime =  timeVal.tv_sec;
    ustime = timeVal.tv_usec;
}

void encoder(EncoderPacket* p) {
    static const unsigned int CNT_PER_REV = 2400;
    if (p == NULL) return;
    p->motorSpeed = motor.getSpeed();
    p->position = motor.getPosition();
    p->raw_cnt = motor.getCnt();
    p->rev_cnt = p->raw_cnt / CNT_PER_REV;
}
