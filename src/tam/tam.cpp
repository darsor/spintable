#include "tam.h"
#include <cstdio>
#include <wiringPi.h>

Tam::Tam(int pinBase, int addr): pinBase(pinBase), addr(addr) {
    try {
        ads1115Setup(pinBase, addr);
        //wiringPiSetup();
    } catch (...) {}
}

uint16_t Tam::getData(int channel) {
	if (channel >= 0 && channel <= 3) {
		channel += pinBase;
	} else {
        printf("%s \n", "Invalid output channel");
        return 0;
    }

	return (uint16_t) analogRead(channel);
}
