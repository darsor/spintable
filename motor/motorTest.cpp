#include "dcmotor.h"
#include <unistd.h>
#include <cmath>
#include <iostream>

using namespace std;

int main() {
	int addr = 0x60;
	int freq = 1600;
	int speed = 0;   
    int inc = 0;
	int speedOld = 0;

	DCMotor myMotor(addr, freq);

	while (true) {
		cout << "Input speed (from -255 to 255): ";
		cin >> speed;
		if (speed > 255 ) {
            speed = 255;
        } else if (speed < -255) {
            speed = -255;
        }

        if (speed < speedOld) inc = -1;
        else inc = 1;

        for (int i=speedOld; i != speed; i += inc) {
            if (i < 0) myMotor.run(BACKWARD);
            else myMotor.run(FORWARD);

            myMotor.setSpeed(abs(i));
            usleep(10000);
        }
		
		speedOld = speed;
        if (speed == 0) break;
	}

    myMotor.run(RELEASE);

	return 0;
}
