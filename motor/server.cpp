#include "gps/gps.h"
#include "motor/dcmotor.h"
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
    uint32_t length = 18;
    uint16_t id = 1;
    uint32_t gpsTime;
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
} tPacket;

struct encoderPacket {
    uint32_t length = 0; // TODO: length
    uint16_t id = 2;
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
    // TODO: define encoder packet
} ePacket;

struct motorPacket {
    int16_t speed;
    uint16_t id;
} mPacket;

// report errors
void error(const char *msg);

// open TCP/IP connection for COSMOS
int cosmosConnect(sockaddr_in &serv_addr, sockaddr_in &cli_addr, int &socketfd);
void cosmosDisconnect(int &bindSocket, int &connectionSocket);

// gather data from various sensors
void gpsTimestamp(timePacket &p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);
// TODO: get encoder data

// convert data to network byte order
void convertTimeData(timePacket &p, char buffer[18]);
// TODO: convert encoder data

// send packets
void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket);
// TODO: send encoder packet

// receive motor packet
void recvPacket(motorPacket &p, int &bindSocket, int &connectionSocket);

// control motor
void setSpeed(DCMotor &motor, motorPacket &p);

int main() {

    // set up wiringPi
    wiringPiSetup();
    std::system("gpio edge 1 rising");

    // initialize packets and sockets
    struct sockaddr_in servAddr, cliAddr;
    int bindSocket, connectionSocket;

    // initialize devices
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
    }

    // establish connection with COSMOS
    connectionSocket = cosmosConnect(servAddr, cliAddr, bindSocket);

    while (true) {

        // get timestamps and send time packet
        waitForInterrupt (1, 2000);
        gpsTimestamp(tPacket);
        systemTimestamp(tPacket.sysTimeSeconds, tPacket.sysTimeuSeconds);
        sendTimePacket(tPacket, connectionSocket, bindSocket);

        // every second, do this 50 times
        for (int j=0; j<50; j++) {
            systemTimestamp(ePacket.sysTimeSeconds, ePacket.sysTimeuSeconds);
            // TODO: get encoder data and send encoder packet
            usleep(10000); // TODO: fine tune the delay
        }
    }
    return 0;
}

PI_THREAD (motorControl) {
    DCMotor motor(2, 0x60, 1600);
    int speed = 0;
    while (true) {
        piLock(0);
        speed = mPacket.speed;
        piUnlock(0);
        if (mPacket.speed != motor.getSpeed()) {
            motor.setGradSpeed(mPacket.speed);
        }
        usleep(500000);
    }
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

void sendTimePacket(timePacket &p, int &connectionSocket, int &bindSocket) {
    static char timeBuffer[18];
    convertTimeData(p, timeBuffer);
    if (send(connectionSocket, timeBuffer, sizeof(timeBuffer), 0) < 0) {
        perror("ERROR on send");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
}

void recvmPacket(int &bindSocket, int &connectionSocket) {
    static char motorBuffer[4];
    if (receive(connectionSocket, motorBuffer, sizeof(motorBuffer), 0) < 0) {
        perror("ERROR on receive");
        cosmosDisconnect(bindSocket, connectionSocket);
        exit(1);
    }
    piLock(0);
    mPacket.speed = *((int16_t *) motorBuffer);
    piUnlock(0);
}

void setSpeed(DCMotor &motor, motorPacket &p) {
    static int speed = 0;
    if (speed != p.speed) {
        speed = p.speed;
        motor.setGradSpeed(speed);
    }
}
