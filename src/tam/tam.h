//Header file for the TAM class.
#ifndef TAM_H
#define TAM_H

#include<ads1115.h>
#include<stdint.h>

class Tam {
    public:
        Tam(int pinBase, int addr);     // default addr is 0x48
        uint16_t getData(int channel);  // channel == 0, 1, 2, or 3.  
    private:
        int pinBase;
        int addr;
};

#endif
