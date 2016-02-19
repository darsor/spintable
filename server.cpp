#include "gps/gps.h"
#include "tam/ADS1115.h"
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
    uint16_t length = sizeof(sensorPacket);
    uint16_t id = 2;
    // TODO: determine data returned by IMU
    uint16_t tamA = 0;
    uint16_t tamB = 0;
    uint16_t tamC = 0;
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
void imu(sensorPacket &p); // TODO: implement this
void tam(sensorPacket &p); //TODO: implement this
void camera(cameraPacket &p); // TODO: implement this
void gpsTimestamp(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// convert data to network standard
void convertTimeData(timePacket &p, char buffer[sizeof(struct timePacket)]);
void convertSensorData(sensorPacket &p, char buffer[sizeof(struct sensorPacket)]); // TODO: implement this

// send packets
void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket);
void sendSensorPacket(sensorPacket &p, int &connectionSocket, int &bindSocket);
void sendCameraPacket(cameraPacket &p, int &connectionSocket, int &bindSocket);

int main() {

    wiringPiSetup();
    std::system("gpio edge 1 rising");

    struct timePacket tPacket;
    struct sensorPacket sPacket;
    struct cameraPacket cPacket;
    struct sockaddr_in servAddr, cliAddr;
    int bindSocket, connectionSocket;

    connectionSocket = cosmosConnect(servAddr, cliAddr, bindSocket);

    while (true) {

        // get timestamps and send time packet
        waitForInterrupt (1, 2000);
        gpsTimestamp(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        printf("GPS time: %u        System time: %u.%u\n", tPacket.gpsTime, tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        sendTimePacket(tPacket, connectionSocket, bindSocket);

        for (int i=0; i<10; i++) {

            for (int j=0; j<5; j++) {
                systemTimestamp(sPacket.sysTimeSeconds, sPacket.sysTimeuSeconds);
//                imu(sPacket);
                tam(sPacket);
//                sendSensorPacket(sPacket, connectionSocket, bindSocket);
//                usleep(20000); // TODO: fine tune the delay
            }

            // TODO: possibly spawn a separate thread for camera?
            // it might take longer than we want
//            camera(cPacket);
//            sendCameraPacket(cPacket, connectionSocket, bindSocket);
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

void convertTimeData(timePacket &p, char buffer[sizeof(struct timePacket)]) {
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

void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket) {
    static char timeBuffer[sizeof(struct timePacket)];
    convertTimeData(p, timeBuffer);
    if (send(connectionSocket, timeBuffer, sizeof(timeBuffer), 0) < 0) {
        perror("ERROR on send");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
}

void sendSensorPacket(sensorPacket &p, int &connectionSocket, int &bindSocket) {
    static char sensorBuffer[sizeof(struct sensorPacket)];
//    convertSensorData(p, sensorBuffer);
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
