#include "packets.h"
#include <arpa/inet.h>
#include <cstring>

Packet::Packet(const uint32_t length, const uint16_t id):
    length(length), id(id) {
}
Packet::~Packet() {}
void Packet::convert(char* buffer) {}

TimePacket::TimePacket(): Packet(TIME_PKT_SIZE, 1) {}

void TimePacket::convert(char* buffer) {
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

SensorPacket::SensorPacket(): Packet(SENSOR_PKT_SIZE, 2) {}

void SensorPacket::convert(char* buffer) {
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

CameraPacket::CameraPacket(): Packet(CAM_PKT_SIZE, 3) {}

void CameraPacket::convert(char* buffer) {
    uint16_t u16;
    uint32_t u32;
    u32 = htonl(length);
    memcpy(buffer+0, &u32, 4);
    u16 = htons(id);
    memcpy(buffer+4, &u16, 2);
    memcpy(buffer+6, pBuffer, 102400);
    u32 = htonl(sysTimeSeconds);
    memcpy(buffer+102406, &u32, 4);
    u32 = htonl(sysTimeuSeconds);
    memcpy(buffer+102410, &u32, 4);
};

EncoderPacket::EncoderPacket(): Packet(ENC_PKT_SIZE, 4) {}

void EncoderPacket::convert(char* buffer) {
    static uint16_t u16;
    static uint32_t u32;
    static int32_t i32;
    static float f;
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
    u32 = htonl(rev_cnt);
    memcpy(buffer+26, &u32, 4);
};
