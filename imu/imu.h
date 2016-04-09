#ifndef IMU_H
#define IMU_H

#include <cstdint>

typedef int ComPortHandle;
typedef unsigned char Byte;

struct ImuSettings {
    uint16_t decimationValue;
    uint16_t conditioningFlags;
    uint8_t accelFilterWidth;
    uint8_t magFilterWidth;
    uint16_t upCompensation;
    uint16_t northCompensation;
    uint8_t bandwidthPower;
    uint16_t reserved;
};

struct ImuSettingsPacket {
    char sync1 = 0x75;
    char sync2 = 0x65;
    char descSet = 0x0c;
    char payloadLength = 0x10;
    char fieldLength = 0x10;
    char fieldDescriptor = 0x35;
    char function; // 0x01 - apply new settings, 0x02 - read current settings, 0x03 - save current settings as startup settings
    struct ImuSettings settings;
    char checksum1;
    char checksum2;
};

class Imu{
    private:
        unsigned char dataCommand = (char) 0xCB;
        unsigned char quatCommand = (char) 0xDF;
        ComPortHandle comPort;
    public:
        Imu();
        ~Imu();
        void printHexByte(char byte);
        void calcChecksum(char* data, int size, uint16_t* checksum);
        int readComPort(unsigned char* bytes, int size);
        int writeComPort(unsigned char* bytes, int size);
        void sendCmd(char* data, int size, char* reply, int reply_size);
        void getdata(Byte (&data)[43]);
            /* Writes the following data to the 43-byte buffer
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
             */
        void getQuaternion (Byte (&data)[23]);
            /* Writes the following data to the 23-byte buffer
             *  Byte  1     0xDF    Command Echo
             *  Bytes 2-5   q0      (IEEE-754 Floating Point)
             *  Bytes 6-9   q1      (IEEE-754 Floating Point)
             *  Bytes 10-13 q2      (IEEE-754 Floating Point)
             *  Bytes 14-17 q3      (IEEE-754 Floating Point)
             *  Bytes 18-21 Timer   32-bit Unsigned Integer
             *  Bytes 22-23 Checksum
             */
};

#endif
