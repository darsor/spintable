#include "dcmotor.h"
#include <cstdio>

DCMotor::DCMotor(int addr, int freq) : pwm(addr) {
	pwmPin = 2;  // for motor 2
	in1Pin = 4;
	in2Pin = 3;

	i2cAddr = addr; // default addr on HAT
    pwm.setPWMFreq(freq); // default @1600Hz PWM freq
}

void DCMotor::run(int command) {
	if (command == FORWARD) {
		setPin(in1Pin, 1);
        setPin(in2Pin, 0);
	}
	if (command == BACKWARD) {
		setPin(in1Pin, 0);
		setPin(in2Pin, 1);
	}
	if (command == RELEASE) {
		setPin(in1Pin, 0);
		setPin(in2Pin, 0);
	}
}

void DCMotor::setSpeed(int speed) {
	if (speed < 0) speed = 0;
	if (speed > 255) speed = 255;
	pwm.setPWM(pwmPin, 0, speed * 16);
}

void DCMotor::setPin(int pin, int value) {
	if (pin < 0 || pin > 15) {
        printf("ERROR: pin must be between 0 and 15\n");
        return;
    }
	if(value != 0 && value != 1) {
        printf("ERROR: value must be 0 or 1\n");
        return;
    }
	if (value == 0) pwm.setPWM(pin, 0, 4096);
	if (value == 1) pwm.setPWM(pin, 4096, 0);
}
