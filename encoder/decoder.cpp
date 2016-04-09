//Implementation file for the Decoder Class
#include <wiringPiSPI.h>
#include "decoder.h"
#include <cstdlib>
#include <cstdio>
#include <mutex>
#include <unistd.h>

std::mutex rw_mutex;

//Decoder constructor
Decoder::Decoder(){
    system("gpio load spi");// loads the spi device on the RasPi
    wiringPiSPISetup(0,1000000);//Sets up the spi device
//  if (wiringPiSPISetup(0,1000000) < 0)
//      perror("ERROR opening device");
    data[0]=0x88; // hexadecimal codes required for Decoder setup
    data[1]=0x01;
    wiringPiSPIDataRW(0, data, 2);//writes to the spi device.
    //clearCntr();
}

//Clears the count on the Decoder
void Decoder::clearCntr(){
    rw_mutex.lock();
    data[0]=0x20;// hexadecimal code to clear the Decoder
    wiringPiSPIDataRW(0, data, 1);
    rw_mutex.unlock();
}

//Reads the count coming from the Decoder.  The first byte is the
//command to read data.  The following zeros are needed to collect
//the date because when using SPI, you can only read after you 
//write something.
uint32_t Decoder::readCntr(){
    rw_mutex.lock();
    data[0]=0x60; 
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    data[4]=0x00;

    wiringPiSPIDataRW(0, data, 5);

    uint32_t count  = ((uint32_t) data[1] << 24) + ((uint32_t) data[2] << 16);
    count += ((uint32_t) data[3] << 8)  +  (uint32_t) data[4];
    rw_mutex.unlock();

    return count; 
} 
