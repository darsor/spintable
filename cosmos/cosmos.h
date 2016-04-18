#ifndef COSMOS_H
#define COSMOS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct timePacket {
    uint32_t length = 18;
    uint16_t id = 1;
    float gpsTime;
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
};

struct sensorPacket {
    uint32_t length = 86;
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
    uint32_t length = 102414;
    uint16_t id = 3;
    unsigned char pBuffer[102400];
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
};

struct encoderPacket {
    uint32_t length = 22;
    uint16_t id = 4;
    uint32_t sysTimeSeconds;
    uint32_t sysTimeuSeconds;
    float motorSpeed;
    float position;
};

class Cosmos {

    public:
        Cosmos(int port);
        ~Cosmos();
        void cosmosConnect();
        void cosmosDisconnect();
        void acceptConnection();
        int sendPacket(char* buffer, int size);
        int recvPacket(char* buffer, int size);

    private:
        int connectionSocket;
        int bindSocket;
        int port;
        unsigned int clilen;
        sockaddr_in serv_addr;
        sockaddr_in cli_addr;
};

#endif
