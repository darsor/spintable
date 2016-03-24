#include "cosmos/cosmos.h"
#include "gps/gps.h"
#include "motor/dcmotor.h"
//TODO #include "encoder/decoder.h"
#include <wiringPi.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>

// gather data from various sensors
void gps(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);
void encoder(encoderPacket &p);

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[18]);
void convertEncoderData(encoderPacket &p, char buffer[18]);

// send packets
int sendTimePacket(timePacket &p, Cosmos &cosmos);
int sendEncoderPacket(encoderPacket &p, Cosmos &cosmos);

// control motor
void setSpeed(DCMotor &motor, motorPacket &p);

// initialize COSMOS
// this is global for now so that the motor can access it
Cosmos cosmos(8321);

PI_THREAD (motorControl) {
    static char motorBuffer[4];
    static motorPacket mPacket;
    static DCMotor motor(2, 0x60, 1600);
    int16_t speed;
    uint16_t id;
    while (true) {
        while (cosmos.recvPacket(motorBuffer, 4) != 0) { // keep trying until the receive works
            usleep(10000); // don't overload processor if this starts looping
        }
        mPacket.speed = ntohs(*((int16_t*)motorBuffer));
        mPacket.id = ntohs(*((uint16_t*)(motorBuffer+2)));
        if (mPacket.id == 1)
            if (mPacket.speed != motor.getSpeed())
                motor.setGradSpeed(mPacket.speed);
        usleep(20000);
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
//      gps(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        if (sendTimePacket(tPacket, cosmos) != 0) {
            printf("lost connection to COSMOS\n");
            return 1;
        }

        // every second, do this 50 times
        for (int j=0; j<50; j++) {
            systemTimestamp(ePacket.sysTimeSeconds, ePacket.sysTimeuSeconds);
                // NOTE: always get the timestamp right before reading the encoder
                // (it's needed to calculate the speed)
//          encoder(ePacket);
//          sendEncoderPacket(ePacket, cosmos);
            usleep(10000); // TODO: fine tune the delay
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
    static Decoder decoder;
    static unsigned int i;
    static double times[4];
    static uint32_t ticks[4];
    ticks[i] = decoder.getQuadCount();
    times[i] = p.sysTimeSeconds + (p.sysTimeuSeconds/1000000.0);

    // TODO: this is untested
    p.motorHz = ( (ticks[(i+1)%4] - ticks[i]) / (times[(i+1)%4] - times[i]) ) / 4915.2;
    
    if (++i > 3) i = 0;
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

void convertEncoderData(encoderPacket &p, char buffer[18]) {
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(p.length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4,  &u16, 2);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+6,  &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+10, &u32, 4);
    u32 = htonl(p.motorHz);
    memcpy(buffer+14, &u32, 4);
}

int sendTimePacket(timePacket &p, Cosmos &cosmos) {
    static char timeBuffer[18];
    convertTimeData(p, timeBuffer);
    return cosmos.sendPacket(timeBuffer, sizeof(timeBuffer));
}

int sendEncoderPacket(encoderPacket &p, Cosmos &cosmos) {
    static char encoderBuffer[18];
    convertEncoderData(p, encoderBuffer);
    return cosmos.sendPacket(encoderBuffer, sizeof(encoderBuffer));
}

void setSpeed(DCMotor &motor, motorPacket &p) {
    static int speed = 0;
    if (speed != p.speed) {
        speed = p.speed;
        motor.setGradSpeed(speed);
    }
}
