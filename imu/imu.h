#ifndef IMU_H
#define IMU_H

typedef int ComPortHandle;
typedef unsigned char Byte;

class Imu{
    private:
        unsigned int command = 0xCF;
        unsigned char ccommand = (char) command;
        ComPortHandle comPort;
    public:
        Imu();
        ~Imu();
        void getdata(Byte (&data)[31]);
        /* Writes sensor telemetry to 'data' in the following way:
         *   Byte 1: i    0xCF      
         *   Bytes 2-5:   Roll      float
         *   Bytes 6-9:   Pitch     float
         *   Bytes 10-13: Yaw       float
         *   Bytes 14-17: AngRateX  float
         *   Bytes 18-21: AngRateY  float
         *   Bytes 22-25: AngRateZ  float
         *   Bytes 26-29: Timer     unsigned int
         *   Bytes 30-31: Checksum
         */
};

#endif
