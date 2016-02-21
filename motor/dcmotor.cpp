#include "motor.h"
#include <cstdio>

DCMotor::DCMotor(int addr, int freq) : pwm(addr) {
	pwmPin = 8;  // for motor 0
	in1Pin = 9;
	in2Pin = 10;

	i2cAddr = addr; // default addr on HAT
    pwm.setPWMFreq(freq); // default @1600Hz PWM freq
}

void DCMotor::run(int command) {
	if (command == FORWARD) {
		setPin(IN2pin, 0);
		setPin(IN1pin, 1);
	}
	if (command == BACKWARD) {
		setPin(IN1pin, 0);
		setPin(IN2pin, 1);
	}
	if (command == RELEASE) {
		setPin(IN1pin, 0);
		setPin(IN2pin, 0);
	}
}

void DCMotor::setSpeed(int speed) {
	if (speed < 0) speed = 0;
	if (speed > 255) speed = 255;
	pwm.setPWM(pwmPIN, 0, speed * 16);
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
