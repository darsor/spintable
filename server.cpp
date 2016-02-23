#include "gps/gps.h"
#include <iostream>
#include "tam/ADS1115.h"
#include "imu/imu.h"
#include <wiringPi.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>

struct timePacket {
    uint16_t length = 16;
    uint16_t id = 1;
    uint32_t gpsTime;
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
};

struct sensorPacket {
    uint16_t length = 84;
    uint16_t id = 2;
    uint16_t tamA = 0;
    uint16_t tamB = 0;
    uint16_t tamC = 0;
    unsigned char imuData[43] = {0};
    /* This buffer receives the following data:
     *  Byte  1     0xCB     Command Echo
     *  Bytes 2-5   AccelX   (IEEE-754 Floating Point)
     *  Bytes 6-9   AccelY   (IEEE-754 Floating Point)
     *  Bytes 10-13 AccelZ   (IEEE-754 Floating Point)
     *  Bytes 14-17 AngRateX (IEEE-754 Floating Point)
     *  Bytes 18-21 AngRateY (IEEE-754 Floating Point)
     *  Bytes 22-25 AngRateZ (IEEE-754 Floating Point)
     *  Bytes 26-29 MagX     (IEEE-754 Floating Point)
     *  Bytes 30-33 MagY     (IEEE-754 Floating Point)
     *  Bytes 34-37 MagZ     (IEEE-754 Floating Point)
     *  Bytes 38-41 Timer    32-bit Unsigned Integer
     *  Bytes 42-43 Checksum
     * NOTE: this data will be parsed by COSMOS when it arrives
     */
    unsigned char imuQuat[23] = {0};
    /* This buffer receives the following quaternion data:
     *  Byte  1     0xDF    Command Echo
     *  Bytes 2-5   q0      (IEEE-754 Floating Point)
     *  Bytes 6-9   q1      (IEEE-754 Floating Point)
     *  Bytes 10-13 q2      (IEEE-754 Floating Point)
     *  Bytes 14-17 q3      (IEEE-754 Floating Point)
     *  Bytes 18-21 Timer   32-bit Unsigned Integer
     *  Bytes 22-23 Checksum
     * NOTE: this data will be parsed by COSMOS when it arrives
     */
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
};

struct cameraPacket {
    uint16_t length = sizeof(cameraPacket);
    uint16_t id = 3;
    unsigned char buffer[10]; // TODO: determine size needed for buffer
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
};

// report errors
void error(const char *msg);

// open TCP/IP connection for COSMOS
int cosmosConnect(sockaddr_in &serv_addr, sockaddr_in &cli_addr, int &socketfd);
void cosmosDisconnect(int &bindSocket, int &connectionSocket);

// gather data from various sensors
void imu(sensorPacket &p);
void tam(sensorPacket &p); //TODO: implement this
void camera(cameraPacket &p); // TODO: implement this
void gpsTimestamp(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[16]);
void convertSensorData(sensorPacket &p, char buffer[84]);

// send packets
void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket);
void sendSensorPacket(sensorPacket &p, int &connectionSocket, int &bindSocket);
void sendCameraPacket(cameraPacket &p, int &connectionSocket, int &bindSocket);

int main() {

    // set up wiringPi
    wiringPiSetup();
    std::system("gpio edge 1 rising");

    // initialize packets and sockets
    struct timePacket tPacket;
    struct sensorPacket sPacket;
    struct cameraPacket cPacket;
    struct sockaddr_in servAddr, cliAddr;
    int bindSocket, connectionSocket;

    // initialize devices
    imu(sPacket);
//  tam(sPacket);

    // establish connection with COSMOS
    connectionSocket = cosmosConnect(servAddr, cliAddr, bindSocket);

    while (true) {

        // get timestamps and send time packet
        waitForInterrupt (1, 2000);
        gpsTimestamp(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        sendTimePacket(tPacket, connectionSocket, bindSocket);

        // every second, do this 10 times
        for (int i=0; i<10; i++) {

            // every second, do this 10*5=50 times
            for (int j=0; j<5; j++) {
                systemTimestamp(sPacket.sysTimeSeconds, sPacket.sysTimeuSeconds);
                imu(sPacket);
//              tam(sPacket);
                sendSensorPacket(sPacket, connectionSocket, bindSocket);
                usleep(10000); // TODO: fine tune the delay
            }

            // TODO: possibly spawn a separate thread for camera?
            // it might take longer than we want
//          camera(cPacket);
//          sendCameraPacket(cPacket, connectionSocket, bindSocket);
            systemTimestamp(cPacket.sysTimeSeconds, cPacket.sysTimeuSeconds);
        }
    }

    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int cosmosConnect(sockaddr_in &serv_addr, sockaddr_in &cli_addr, int &socketfd) {

    int portno = 8321;

    // ignore sigpipe signal - don't stop program when writing to closed socket
    // (it will be handled instead)
    signal(SIGPIPE, SIG_IGN);

    // open socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd<0) error("ERROR opening socket");

    // clear and set up server address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // bind socket
    if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    // listen on socket
    listen(socketfd, 1);
    printf("listening on port %d\n", portno);

    // accept a connection
    unsigned int clilen = sizeof(cli_addr);
    int newsocketfd = accept(socketfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsocketfd<0) error("ERROR on accept");

    printf("accepted connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    return newsocketfd;
}

void cosmosDisconnect(int &bindSocket, int &connectionSocket) {
    close(bindSocket);
    close(connectionSocket);
}

void imu(sensorPacket &p) {
    static Imu imuSensor;
    imuSensor.getdata(p.imuData);
    imuSensor.getQuaternion(p.imuQuat);
}

void tam(sensorPacket &p) { // TODO: this doesn't work
    static ADS1115 Tam(0x48);
    //printf("Tam connected: %s\n", Tam.testConnection() ? "yes" : "no");
    Tam.setMultiplexer(0);
    p.tamA = Tam.getConversion();
    Tam.setMultiplexer(1);
    p.tamB = Tam.getConversion();
    Tam.setMultiplexer(2);
    p.tamC = Tam.getConversion();
    printf("ADS A0: %-10u A1: %-10u A2: %-10u\n", p.tamA, p.tamB, p.tamB);
}

void gpsTimestamp(timePacket &p) {
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

void convertTimeData(timePacket &p, char buffer[16]) {
    uint16_t u16;
    uint32_t u32;
    u16 = htons(p.length);
    memcpy(buffer+0,  &u16, 2);
    u16 = htons(p.id);
    memcpy(buffer+2,  &u16, 2);
    u32 = htonl(p.gpsTime);
    memcpy(buffer+4,  &u32, 4);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+8,  &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+12, &u32, 4);
}

void convertSensorData(sensorPacket &p, char buffer[84]) {
    uint16_t u16;
    uint32_t u32;
    u16 = htons(p.length);
    memcpy(buffer+0,  &u16, 2);
    u16 = htons(p.id);
    memcpy(buffer+2,  &u16, 2);
    u16 = htons(p.tamA);
    memcpy(buffer+4,  &u16, 2);
    u16 = htons(p.tamB);
    memcpy(buffer+6,  &u16, 2);
    u16 = htons(p.tamC);
    memcpy(buffer+8,  &u16, 2);
    memcpy(buffer+10, &p.imuData, 43);
    memcpy(buffer+53, &p.imuQuat, 23);
    u32 = htonl(p.sysTimeSeconds);
    memcpy(buffer+76, &u32, 4);
    u32 = htonl(p.sysTimeuSeconds);
    memcpy(buffer+80, &u32, 4);
}

void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket) {
    static char timeBuffer[16];
    convertTimeData(p, timeBuffer);
    if (send(connectionSocket, timeBuffer, sizeof(timeBuffer), 0) < 0) {
        perror("ERROR on send");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
}

void sendSensorPacket(sensorPacket &p, int &connectionSocket, int &bindSocket) {
    static char sensorBuffer[84];
    convertSensorData(p, sensorBuffer);
    if (send(connectionSocket, sensorBuffer, sizeof(sensorBuffer), 0) < 0) {
        perror("ERROR on send");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
}

void sendCameraPacket(cameraPacket &p, int &connectionSocket, int &bindSocket) {
    static char cameraBuffer[sizeof(struct sensorPacket)];
    // TODO: format the buffer or something to send camera data
    if (send(connectionSocket, cameraBuffer, sizeof(cameraBuffer), 0) < 0) {
        perror("ERROR on send");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
}
