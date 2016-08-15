// header file for the Decoder Class
#ifndef DECODER_H
#define DECODER_H

#include <wiringPiSPI.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

class Decoder{
    private:
        unsigned char data[5];
    public:
        Decoder();
        int32_t readCntr();
        void clearCntr();
};

#endif
