#ifndef IMU_H
#define IMU_H

typedef int ComPortHandle;
typedef unsigned char Byte;

class Imu{
    private:
        unsigned char dataCommand = (char) 0xCB;
        unsigned char quatCommand = (char) 0xDF;
        ComPortHandle comPort;
    public:
        Imu();
        ~Imu();
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
