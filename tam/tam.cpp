//Implementation file for the TAM class.  Necessary header files
//are tam.h and ads1115.h.  ads1115.h is included within tam.h
#include "tam.h"

Tam::Tam() {
    ads1115Setup(100, 0x48);
    wiringPiSetup();
}

uint16_t Tam::getData(int channel) {
	if (channel >= 0 && channel <= 3) {
		channel += 100;
	} else {
        printf("%s \n", "Invalid output channel");
        return 0;
    }

	return (uint16_t) analogRead(channel);
}
