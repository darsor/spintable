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

	DCMotor myMotor(3, addr, freq);

	while (true) {
		cout << "Input speed (from -255 to 255): ";
		cin >> speed;

        myMotor.setGradSpeed(speed);

        if (speed == 0) break;
	}

    myMotor.run(RELEASE);

	return 0;
}
