#include "packets.h"
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>

Packet::Packet(const uint32_t length, const uint16_t id, bool cmd):
    length(length), id(id) {
    if (cmd) buffer = new unsigned char[length];
    else buffer = nullptr;
}
Packet::Packet(const Packet& that): length(that.length), id(that.id) {
    buffer = nullptr;
}
Packet::~Packet() {
    if (buffer) delete[] buffer;
}
Packet& Packet::operator=(const Packet& that) {
    if (this != &that)
    {
        if (buffer) delete[] buffer;
        buffer = nullptr;
        length = that.length;
        id = that.id;
    }
    return *this;
}
void Packet::convert() { printf("Virtual convert() called. That's a problem\n"); }

TimePacket::TimePacket(): Packet(TIME_PKT_SIZE, TIME_PKT_ID) {}

void TimePacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    uint16_t u16;
    uint32_t u32;
    float f;
    u32 = htonl(length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(id);
    memcpy(buffer+4,  &u16, 2);
    f = gpsTime;
    endianSwap(f);
    memcpy(buffer+6,  &f, 4);
    u32 = htonl(sysTimeSeconds);
    memcpy(buffer+10,  &u32, 4);
    u32 = htonl(sysTimeuSeconds);
    memcpy(buffer+14, &u32, 4);
};

SensorPacket::SensorPacket(): Packet(SENSOR_PKT_SIZE, SENSOR_PKT_ID) {}

void SensorPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(id);
    memcpy(buffer+4,  &u16, 2);
    u16 = htons(tamA);
    memcpy(buffer+6,  &u16, 2);
    u16 = htons(tamB);
    memcpy(buffer+8,  &u16, 2);
    u16 = htons(tamC);
    memcpy(buffer+10,  &u16, 2);
    memcpy(buffer+12, imuData, 43);
    memcpy(buffer+55, imuQuat, 23);
    u32 = htonl(sysTimeSeconds);
    memcpy(buffer+78, &u32, 4);
    u32 = htonl(sysTimeuSeconds);
    memcpy(buffer+82, &u32, 4);
};

CameraPacket::CameraPacket(): Packet(CAM_PKT_SIZE, CAM_PKT_ID) {
    pBuffer = new unsigned char[76800];
}

CameraPacket::~CameraPacket() {
    delete[] pBuffer;
}

void CameraPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(length);
    memcpy(buffer+0, &u32, 4);
    u16 = htons(id);
    memcpy(buffer+4, &u16, 2);
    memcpy(buffer+6, pBuffer, 76800);
    u32 = htonl(sysTimeSeconds);
    memcpy(buffer+76806, &u32, 4);
    u32 = htonl(sysTimeuSeconds);
    memcpy(buffer+76810, &u32, 4);
};

EncoderPacket::EncoderPacket(): Packet(ENC_PKT_SIZE, ENC_PKT_ID) {}

void EncoderPacket::convert() {
    static uint16_t u16;
    static uint32_t u32;
    static int32_t i32;
    static float f;
    if (!buffer) buffer = new unsigned char[length];
    u32 = htonl(length);
    memcpy(buffer+0,  &u32, 4);
    u16 = htons(id);
    memcpy(buffer+4,  &u16, 2);
    u32 = htonl(sysTimeSeconds);
    memcpy(buffer+6,  &u32, 4);
    u32 = htonl(sysTimeuSeconds);
    memcpy(buffer+10, &u32, 4);
    i32 = htonl(raw_cnt);
    memcpy(buffer+14, &i32, 4);
    f = motorSpeed;
    endianSwap(f);
    memcpy(buffer+18, &f, 4);
    f = position;
    endianSwap(f);
    memcpy(buffer+22, &f, 4);
    i32 = htonl(rev_cnt);
    memcpy(buffer+26, &i32, 4);
};

CameraPowerCmd::CameraPowerCmd(): Packet(CAM_CMD_SIZE, CAM_CMD_ID) {}

void CameraPowerCmd::convert() {
    static uint16_t u16;
    memcpy(&u16, buffer+6, sizeof(u16));
    state = ntohs(u16);
}

SetSpeedCmd::SetSpeedCmd() : Packet(MOTOR_SET_SPEED_SIZE, MOTOR_SET_SPEED_ID) {}

void SetSpeedCmd::convert() {
    static int16_t i16;
    memcpy(&i16, buffer+6, sizeof(i16));
    speed = ntohs(i16);
}

SetAbsPosCmd::SetAbsPosCmd() : Packet(MOTOR_ABS_POS_SIZE, MOTOR_ABS_POS_ID) {}

void SetAbsPosCmd::convert() {
    memcpy(&position, buffer+6, sizeof(position));
    endianSwap(position);
}

SetRevPosCmd::SetRevPosCmd() : Packet(MOTOR_REV_POS_SIZE, MOTOR_REV_POS_ID) {}

void SetRevPosCmd::convert() {
    memcpy(&position, buffer+6, sizeof(position));
    endianSwap(position);
}
