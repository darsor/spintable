#include "motor.h"
#include <iostream>

using namespace std;

int main() {
	int addr = 0x60;
	int freq = 1600;
	int speed = 0;   
	int speedOld = 0;

	DCMotor myMotor(addr, freq);

	while (true) {
		cout << "Input speed (from -255 to 255): ";
		cin >> speed;
		if (speed > 255 ) {
            speed = 255;
        } else if (speed < -255) {
            speed = -255;
        } else if (speed == 0) {
            break;
        }
		
		if (speed > speedOld) {
			for (int i = speed - speedOld; i > 0; i--) {
				myMotor.run(FORWARD);
				myMotor.setSpeed(speed-i);
				usleep(10000);
			}
		}
		if (speed < speedOld) {
			for (int i = speedOld - speed; i > 0; i--) {
				myMotor.run(BACKWARD);
				myMotor.setSpeed(speed+i);
				usleep(10000);
			}
		}
		speedOld = speed;
	}

    myMotor.run(RELEASE);

	return 0;
}
