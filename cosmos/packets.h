/**********************
 * Packet Definitions *
 **********************/
#ifndef PACKETS_H
#define PACKETS_H

#define TIME_PKT_SIZE           18
#define SENSOR_PKT_SIZE         98
#define CAM_PKT_SIZE            76814
#define ENC_PKT_SIZE            30
#define CAM_CMD_SIZE            8
#define MOTOR_SET_HOME_SIZE     6
#define MOTOR_SET_SPEED_SIZE    8
#define MOTOR_ABS_POS_SIZE      10
#define MOTOR_REV_POS_SIZE      10
#define MOTOR_GOTO_INDEX_SIZE   6

#define TIME_PKT_ID             1
#define SENSOR_PKT_ID           2
#define CAM_PKT_ID              3
#define ENC_PKT_ID              4
#define CAM_CMD_ID              5
#define MOTOR_SET_HOME_ID       9
#define MOTOR_SET_SPEED_ID      10
#define MOTOR_ABS_POS_ID        11
#define MOTOR_REV_POS_ID        12
#define MOTOR_GOTO_INDEX_ID     13

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
        Packet(const Packet& that);
        Packet& operator=(const Packet& that);
        virtual ~Packet();
        virtual void convert();
        uint32_t length;
        uint16_t id;
        unsigned char* buffer;
};

class TimePacket: public Packet {
    public:
        TimePacket();
        void convert();
        float gpsTime;
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
};

class SensorPacket: public Packet {
    public:
        SensorPacket();
        void convert();
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
        float fvm400x;
        float fvm400y;
        float fvm400z;
};

class CameraPacket: public Packet {
    public:
        CameraPacket();
        void convert();
        unsigned char pBuffer[76800];
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
};

class EncoderPacket: public Packet {
    public:
        EncoderPacket();
        void convert();
        uint32_t sysTimeSeconds;
        uint32_t sysTimeuSeconds;
        int32_t raw_cnt;
        float motorSpeed;
        float position;
        int32_t rev_cnt;
};

class CameraPowerCmd: public Packet {
    public:
        CameraPowerCmd();
        void convert();
        uint16_t state;
};

class SetSpeedCmd: public Packet {
    public:
        SetSpeedCmd();
        void convert();
        int16_t speed;
};

class SetAbsPosCmd: public Packet {
    public:
        SetAbsPosCmd();
        void convert();
        float position;
};

class SetRevPosCmd: public Packet {
    public:
        SetRevPosCmd();
        void convert();
        float position;
};

#endif
