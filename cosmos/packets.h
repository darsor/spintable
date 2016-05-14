/**********************
 * Packet Definitions *
 **********************/
#ifndef PACKETS_H
#define PACKETS_H

#define TIME_PKT_SIZE       18
#define SENSOR_PKT_SIZE     86
#define CAM_PKT_SIZE        102414
#define ENC_PKT_SIZE        30

#include <cstdint>

inline void endianSwap(float &f) {
    float temp = f;
    unsigned char* pf = (unsigned char*) &f;
    unsigned char* pt = (unsigned char*) &temp;
    pf[0] = pt[3];
    pf[1] = pt[2];
    pf[2] = pt[1];
    pf[3] = pt[0];
}

class Packet {
    public:
        Packet(const uint32_t length, const uint16_t id);
        virtual ~Packet();
        virtual void convert(char* buffer);
        uint32_t length;
        uint16_t id;
};

class TimePacket: public Packet {
    public:
        TimePacket();
        void convert(char* buffer);
        float gpsTime;
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
};

class SensorPacket: public Packet {
    public:
        SensorPacket();
        void convert(char* buffer);
        uint16_t tamA;
        uint16_t tamB;
        uint16_t tamC;
        unsigned char imuData[43];
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
        unsigned char imuQuat[23];
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

class CameraPacket: public Packet {
    public:
        CameraPacket();
        void convert(char* buffer);
        unsigned char pBuffer[102400];
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
};

class EncoderPacket: public Packet {
    public:
        EncoderPacket();
        void convert(char* buffer);
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
        int32_t raw_cnt;
        float motorSpeed;
        float position;
        uint32_t rev_cnt;
};

#endif
