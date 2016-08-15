#include "packets.h"
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>

Packet::Packet(const uint32_t length, const uint8_t id, bool cmd):
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
    memcpy(buffer+0, &length,     4);
    memcpy(buffer+4, &id,         1);
    memcpy(buffer+5, &gpsTime,    4);
    memcpy(buffer+9, &systemTime, 8);
};

TamPacket::TamPacket(): Packet(TAM_PKT_SIZE, TAM_PKT_ID) {}

void TamPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+ 0, &length,  4);
    memcpy(buffer+ 4, &id,      1);
    memcpy(buffer+ 5, &timeA,   8);
    memcpy(buffer+13, &tamA,    2);
    memcpy(buffer+15, &timeB,   8);
    memcpy(buffer+23, &tamB,    2);
    memcpy(buffer+25, &timeC,   8);
    memcpy(buffer+33, &tamC,    2);
}

ImuPacket::ImuPacket(): Packet(IMU_PKT_SIZE, IMU_PKT_ID) {}

void ImuPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+ 0, &length       ,  4);
    memcpy(buffer+ 4, &id           ,  1);
    memcpy(buffer+ 5, &dataTimestamp,  8);
    memcpy(buffer+13, imuData       , 43);
    memcpy(buffer+56, &quatTimestamp,  8);
    memcpy(buffer+64, imuQuat       , 23);
};

CameraPacket::CameraPacket(): Packet(CAM_PKT_SIZE, CAM_PKT_ID) {
    pBuffer = new unsigned char[76800];
}

CameraPacket::~CameraPacket() {
    delete[] pBuffer;
}

void CameraPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0, &length,    4);
    memcpy(buffer+4, &id,        1);
    memcpy(buffer+5, &timestamp, 8);
    memcpy(buffer+13, pBuffer,   76800);
};

EncoderPacket::EncoderPacket(): Packet(ENC_PKT_SIZE, ENC_PKT_ID) {}

void EncoderPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+ 0, &length,          4);
    memcpy(buffer+ 4, &id,              1);
    memcpy(buffer+ 5, &sysTimeSeconds,  4);
    memcpy(buffer+ 9, &sysTimeuSeconds, 4);
    memcpy(buffer+13, &raw_cnt,         4);
};

HKPacket::HKPacket() : Packet(HK_PKT_SIZE, HK_PKT_ID) {}

void HKPacket::convert() {
    if (!buffer) buffer = new unsigned char[length];
    memcpy(buffer+0 , &length    , 4);
    memcpy(buffer+4 , &id        , 1);
    memcpy(buffer+5 , &timestamp , 8);
    memcpy(buffer+13, &queue_size, 2);
    memcpy(buffer+15, &cpu_temp  , 4);
    memcpy(buffer+19, &cpu_load  , 4);
    memcpy(buffer+23, &mem_usage , 4);
}

CameraPowerCmd::CameraPowerCmd(): Packet(CAM_CMD_SIZE, CAM_CMD_ID) {}

void CameraPowerCmd::convert() {
    memcpy(&state, buffer+5, sizeof(state));
}

SetSpeedCmd::SetSpeedCmd() : Packet(MOTOR_SET_SPEED_SIZE, MOTOR_SET_SPEED_ID) {}

void SetSpeedCmd::convert() {
    memcpy(&speed, buffer+5, sizeof(speed));
}

SetAbsPosCmd::SetAbsPosCmd() : Packet(MOTOR_ABS_POS_SIZE, MOTOR_ABS_POS_ID) {}

void SetAbsPosCmd::convert() {
    memcpy(&position, buffer+5, sizeof(position));
}

SetRevPosCmd::SetRevPosCmd() : Packet(MOTOR_REV_POS_SIZE, MOTOR_REV_POS_ID) {}

void SetRevPosCmd::convert() {
    memcpy(&position, buffer+5, sizeof(position));
}
