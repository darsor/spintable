#include "cosmos/cosmos.h"
#include "gps/gps.h"
#include "camera/camera.h"
#include "tam/tam.h"
#include "imu/imu.h"
#include <wiringPi.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

// gather data from various sensors
void imu(sensorPacket &p);
void tam(sensorPacket &p);
void camera(cameraPacket &p);
void gps(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[18]);
void convertSensorData(sensorPacket &p, char buffer[86]);
void convertCameraData(cameraPacket &p, char buffer[102414]);

// send packets
int sendTimePacket(timePacket &p, Cosmos &cosmos);
int sendCameraPacket(cameraPacket &p, Cosmos &cosmos);
int sendSensorPacket(sensorPacket &p, Cosmos &cosmos);

// initialize cosmos
Cosmos cosmos(8321);

PI_THREAD (cameraControl) {
    static struct cameraPacket cPacket;
    camera(cPacket);
    while (true) {
        systemTimestamp(cPacket.sysTimeSeconds, cPacket.sysTimeuSeconds);
        camera(cPacket);
        if (sendCameraPacket(cPacket, cosmos) != 0) {
            printf("camera packet not sent\n");
            usleep(5000000);
        }
        usleep(40000);
    }
}

int main() {

    // set up wiringPi
    wiringPiSetup();

    // initialize packets
    struct timePacket tPacket;
    struct sensorPacket sPacket;

    // establish connection with COSMOS
    cosmos.acceptConnection();

    // initialize devices
    imu(sPacket);
    tam(sPacket);
    gps(tPacket);

    if (piThreadCreate(cameraControl) != 0) {
        perror("Camera control thread didn't start");
    }

    while (true) {

        // get timestamps and send time packet
        gps(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        if (sendTimePacket(tPacket, cosmos) != 0) {
            printf("Connection with COSMOS lost\n");
            cosmos.acceptConnection();
            printf("Reconnected with COSMOS\n");
        }
        // every second, do this 50 times
        for (int j=0; j<50; j++) {
            systemTimestamp(sPacket.sysTimeSeconds, sPacket.sysTimeuSeconds);
            imu(sPacket); // TODO: put one of these in a thread?
            tam(sPacket); // there's too much delay after the timestamp
            sendSensorPacket(sPacket, cosmos);
//          usleep(2000); // TODO: fine tune the delay
        }
    }
    return 0;
}

void imu(sensorPacket &p) {
    static Imu imuSensor;
    imuSensor.getdata(p.imuData);
    imuSensor.getQuaternion(p.imuQuat);
}

void tam(sensorPacket &p) {
    static Tam tamSensor;
    p.tamA = tamSensor.getData(0);
    p.tamB = tamSensor.getData(1);
    p.tamC = tamSensor.getData(2);
}

void camera(cameraPacket &p) {
    static Camera cam;
    cam.getFrame(p.pBuffer);
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
    float f;
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

void convertSensorData(sensorPacket &p, char buffer[86]) {
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(p.length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4,  &u16, 2);
    u16 = htons(p.tamA);
    memcpy(buffer+6,  &u16, 2);
    u16 = htons(p.tamB);
    memcpy(buffer+8,  &u16, 2);
    u16 = htons(p.tamC);
    memcpy(buffer+10,  &u16, 2);
    memcpy(buffer+12, p.imuData, 43);
    memcpy(buffer+55, p.imuQuat, 23);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+78, &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+82, &u32, 4);
}

void convertCameraData(cameraPacket &p, char buffer[102414]) {
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(p.length);
    memcpy(buffer+0, &u32, 4);
    u16 = htons(p.id);
    memcpy(buffer+4, &u16, 2);
    memcpy(buffer+6, p.pBuffer, 102400);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+102406, &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+102410, &u32, 4);
}

int sendTimePacket(timePacket &p, Cosmos &cosmos) {
    static char timeBuffer[18];
    convertTimeData(p, timeBuffer);
    return cosmos.sendPacket(timeBuffer, sizeof(timeBuffer));
}

int sendSensorPacket(sensorPacket &p, Cosmos &cosmos) {
    static char sensorBuffer[86];
    convertSensorData(p, sensorBuffer);
    return cosmos.sendPacket(sensorBuffer, sizeof(sensorBuffer));
}

int sendCameraPacket(cameraPacket &p, Cosmos &cosmos) {
    static char cameraBuffer[102414];
    convertCameraData(p, cameraBuffer);
    return cosmos.sendPacket(cameraBuffer, sizeof(cameraBuffer));
}
