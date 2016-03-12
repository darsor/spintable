#ifndef TAM_H
#define TAM_H

#include<iostream>
#include<wiringPi.h>
#include<ads1115.h>
#include<stdio.h>
#include<stdint.h>

class Tam{
    public:

        Tam();
        uint16_t getData(int channel); // channel == 0, 1, 2, or 3.  
};

#endif
