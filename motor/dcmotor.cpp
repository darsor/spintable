#include "dcmotor.h"
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <mutex>

std::mutex motor_mutex;

DCMotor::DCMotor(int channel, int addr, int freq) : pwm(addr), decoder() {
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
    pwmSpeed = 0;
    pwmSpeedOld = 0;
    pwm.setPWMFreq(freq); // default @1600Hz PWM freq
    run(RELEASE);
}

DCMotor::~DCMotor() {
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
    pwmSpeed = speed;
}

void DCMotor::setGradSpeed(int speed) {
    if (speed > 255 ) {
        speed = 255;
    } else if (speed < -255) {
        speed = -255;
    }
    //printf("speed changing from %d to %d\n", pwmSpeedOld, speed);

    if (speed < pwmSpeedOld) inc = -1;
    else inc = 1;

    for (int i=pwmSpeedOld; i != speed ; i += inc) {
        //printf("old speed: %d, current speed: %d, new speed: %d\n", pwmSpeedOld, i, speed);
        if (i < 0) {
            run(BACKWARD);
        }
        else { run(FORWARD);
        }

        setSpeed(abs(i));
        usleep(6000);
    }

    if (speed == 0) {
        run(RELEASE);
    }
    pwmSpeedOld = speed;
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

void DCMotor::setHome() {
    decoder.clearCntr();
}

double DCMotor::getSpeed() {
    // TODO: test this
    static struct timeval timeVal;
    static struct timezone timeZone;
    static unsigned int i;
    static double times[4];
    static int32_t ticks[4];
    motor_mutex.lock();
    gettimeofday(&timeVal, &timeZone);
    times[i] =  timeVal.tv_sec + (timeVal.tv_usec/1000000.0);
    ticks[i] = decoder.readCntr();

    degSpeed = ( (ticks[i] - ticks[(i+1)%4]) /
                 (times[i] - times[(i+1)%4]) ) * DEG_PER_CNT;

    if (++i > 3) i = 0;
    motor_mutex.unlock();

    return degSpeed;
}

// return position in degrees from home
double DCMotor::getPosition() {
    int count = decoder.readCntr();
    degPosition = (count % (int) CNT_PER_REV) * DEG_PER_CNT;
    if (count < 0) degPosition += 360;
    return degPosition;
}
