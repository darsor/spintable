#ifndef PACKETS_H
#define PACKETS_H

#define TIME_PKT_ID             0x00
#define TIME_PKT_SIZE           17

#define TAM_PKT_ID              0x10
#define TAM_PKT_SIZE            35

#define IMU_PKT_ID              0x20
#define IMU_PKT_SIZE            87

#define CAM_PKT_ID              0x30
#define CAM_PKT_SIZE            76813
#define CAM_CMD_ID              0x31
#define CAM_CMD_SIZE            7

#define ENC_PKT_ID              0x40
#define ENC_PKT_SIZE            17

#define MOTOR_SET_HOME_ID       0x41
#define MOTOR_SET_HOME_SIZE     5
#define MOTOR_SET_SPEED_ID      0x42
#define MOTOR_SET_SPEED_SIZE    7
#define MOTOR_ABS_POS_ID        0x43
#define MOTOR_ABS_POS_SIZE      9
#define MOTOR_REV_POS_ID        0x44
#define MOTOR_REV_POS_SIZE      9
#define MOTOR_GOTO_INDEX_ID     0x45
#define MOTOR_GOTO_INDEX_SIZE   5

#define HK_PKT_ID               0xFF
#define HK_PKT_SIZE             27

#include <cstdint>

class Packet {
    public:
        Packet(const uint32_t length, const uint8_t id, bool cmd = false);
        Packet(const Packet& that);
        Packet& operator=(const Packet& that);
        virtual ~Packet();
        virtual void convert();
        uint32_t length;
        uint8_t id;
        unsigned char* buffer;
};

class TimePacket: public Packet {
    public:
        TimePacket();
        void convert();
        float gpsTime;
        uint64_t systemTime;
};

class TamPacket: public Packet {
    public:
        TamPacket();
        void convert();
        uint64_t timeA;
        uint16_t tamA;
        uint64_t timeB;
        uint16_t tamB;
        uint64_t timeC;
        uint16_t tamC;
};

class ImuPacket: public Packet {
    public:
        ImuPacket();
        void convert();
        uint64_t dataTimestamp;
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
        uint64_t quatTimestamp;
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
};

class CameraPacket: public Packet {
    public:
        CameraPacket();
        ~CameraPacket();
        void convert();
        uint64_t timestamp;
        unsigned char* pBuffer;
};

class EncoderPacket: public Packet {
    public:
        EncoderPacket();
        void convert();
        uint64_t timestamp;
        int32_t raw_cnt;
};

class HKPacket: public Packet {
    public:
        HKPacket();
        void convert();
        uint64_t timestamp;
        uint16_t queue_size;
        float cpu_temp;
        float cpu_load;
        uint32_t mem_usage;
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
