#include <wiringPiSPI.h>
#include "decoder.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

Decoder::Decoder(){
    system("gpio load spi");// loads the spi device on the RasPi
    wiringPiSPISetup(0,1000000);
//  if (wiringPiSPISetup(0,1000000) < 0)
//      perror("ERROR opening device");
    data[0]=0x88; // hexadecimal codes required for Decoder Setup
    data[1]=0x01;
    wiringPiSPIDataRW(0, data, 2);
    clearCntr();
}

void Decoder::clearCntr(){
    data[0]=0x20;// hexadecimal code to clear the Decoder
    wiringPiSPIDataRW(0, data, 1);
}

uint32_t Decoder::readCntr(){
    data[0]=0x60; 
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    data[4]=0x00;

    wiringPiSPIDataRW(0, data, 5);

    uint32_t count  = ((uint32_t) data[1] << 24) + ((uint32_t) data[2] << 16);
    count += ((uint32_t) data[3] << 8)  +  (uint32_t) data[4];

    return count; 
} 
