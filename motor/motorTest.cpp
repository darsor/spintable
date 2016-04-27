#include "dcmotor.h"
#include <unistd.h>
#include <cmath>
#include <iostream>

using namespace std;

int main() {
	int addr = 0x60;
	int freq = 1600;
	double speed = 0;   
    double position;

	DCMotor myMotor(2, addr, freq);
    myMotor.setHome();

	while (true) {
        /*
		cout << "Input speed (from -255 to 255): ";
		cin >> speed;

        myMotor.setGradSpeed(speed);
        if (speed == 0) break;

        for (int i=0; i<3; i++) {
            printf("position: %-.4f\n", myMotor.getPosition());
            printf("speed: %-.4f\n", myMotor.getSpeed());
        }
        */


        cout << "Input position (from 0 to 360): ";
        cin >> position;
        myMotor.stopPID();
        myMotor.pidPosition(position);
        //printf("position: %-.4f\n", myMotor.getPosition());
        usleep(20000);


        /*
        cout << "Input speed (from -1200 to 1200): ";
        cin >> speed;

        myMotor.stopPID();
        myMotor.pidSpeed(speed);
        */
	}

    myMotor.run(RELEASE);

	return 0;
}
