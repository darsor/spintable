#include "dcmotor.h"
#include <unistd.h>
#include <cstdlib>
#include <cstdio>

DCMotor::DCMotor(int channel, int addr, int freq) : pwm(addr) {
    if (channel == 0) {
        pwmPin = 8;
        in2Pin = 9;
        in1Pin = 10;
    } else if (channel == 1) {
        pwmPin = 13;
        in2Pin = 12;
        in1Pin = 11;
    } else if (channel == 2) {
        pwmPin = 2;
        in2Pin = 3;
        in1Pin = 4;
    } else if (channel == 3) {
        pwmPin = 7;
        in2Pin = 6;
        in1Pin = 5;
    } else printf("ERROR: incorrect motor channel, must be between 0-3\n");

	i2cAddr = addr;
    pwm.setPWMFreq(freq); // default @1600Hz PWM freq
}

DCMotor::~DCMotor() {
    setGradSpeed(0);
    run(RELEASE);
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
    mSpeed = speed;
}

void DCMotor::setGradSpeed(int speed) {
    if (speed > 255 ) {
        speed = 255;
    } else if (speed < -255) {
        speed = -255;
    }

    if (speed < mSpeedOld) inc = -1;
    else inc = 1;

    for (int i=mSpeedOld; i != speed ; i += inc) {
        if (i < 0) run(BACKWARD);
        else run(FORWARD);

        setSpeed(abs(i));
        usleep(8000);
        if (i > speed) break; // sanity check
    }

    mSpeedOld = speed;
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
