// implementation file for the decoder
#include <wiringPiSPI.h>
#include "decoder.h"
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <mutex>

std::mutex rw_mutex;

// decoder constructor
Decoder::Decoder(){
    if (wiringPiSPISetup(0,2000000) < 0) {
        perror("ERROR opening device");
        throw 1;
    }
    rw_mutex.lock();
    data[0]=0x88; // hexadecimal codes required for Decoder setup
    data[1]=0x03;
    //writes to the spi device.
    if (wiringPiSPIDataRW(0, data, 2) < 0) {
        throw 1;
    }
    rw_mutex.unlock();
    //clearCntr();
}

// clears the count on the Decoder
void Decoder::clearCntr(){
    rw_mutex.lock();
    data[0]=0x20;// hexadecimal code to clear the Decoder
    wiringPiSPIDataRW(0, data, 1);
    rw_mutex.unlock();
}

// Reads the count coming from the Decoder.  The first byte is the
// command to read data.  The following zeros are needed to collect
// the data because of the way SPI works
int32_t Decoder::readCntr(){
    rw_mutex.lock();

    data[0]=0x60; 
    if (wiringPiSPIDataRW(0, data, 5) < 0) {
        throw 1;
    }

    int32_t count  = ((int32_t) data[1] << 24) + ((int32_t) data[2] << 16) +
                     ((int32_t) data[3] << 8)  + ((int32_t) data[4]);
    rw_mutex.unlock();

    return count; 
} 
