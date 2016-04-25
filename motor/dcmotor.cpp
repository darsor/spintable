#include "dcmotor.h"
#include "pid.h"
#include <sys/time.h>
#include <unistd.h>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <mutex>

std::mutex speed_mutex;
std::mutex pos_mutex;

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
    runningPID.store(false);
    index_tripped = false;
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
	if (speed < -255) speed = -255;
	if (speed > 255) speed = 255;
    if (speed == 0) {
        run(RELEASE);
    } else if (speed > 0) {
        run(FORWARD);
        pwm.setPWM(pwmPin, 0, speed * 16);
    } else if (speed < 0) {
        run(BACKWARD);
        pwm.setPWM(pwmPin, 0, abs(speed) * 16);
    }
    pwmSpeed = speed;
}

void DCMotor::setGradSpeed(int speed) {
    motor.stopPID();
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
        setSpeed(i);
        usleep(6000);
    }
    setSpeed(speed);
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
    motor.stopPID();
    decoder.clearCntr();
}

// TODO: implement this
void DCMotor::gotoIndex() {
    motor.stopPID();
    motor.setGradSpeed(0);
    /* move this to the constructor
        wiringPiISR(indexPin, INT_EDGE_RISING, &indexISR)
     */
    // index pulses are 75 degrees apart, so the closest index pulse is within 37.5
    // record starting position
    double position = getPosition();
    pidPosition(position);
    // move right 3.75 degrees, 10 times (with delay of about 200ms)
    for (int i=0; i<10; i++) {
        setPos = (setPos + 3.75 > 360) ? setPos + 3.75 - 360 : setPos + 3.75;
        while (setPos != position);
        // if the index pulse was triggered, break the loop
        if (index_tripped) break;
        usleep(200000);
    }
    // if none were triggered, move back to starting position and repeat going left
    if (!index_tripped) {
        setPos = position;
        for (int i=0; i<10; i++) {
            setPos = (setPos - 3.75 < 0) ? setPos - 3.75 + 360 : setPos - 3.75;
            while (setPos != position);
            // if the index pulse was triggered, break the loop
            if (index_tripped) break;
            usleep(200000);
        }
    }
    // move left (or right) one quadrature pulse (0.15 degrees) at a time until the index pulse is triggered
    // record the count the index was triggered on/set setPos/set home 
    printf("gotoIndex called. Function not yet implemented\n");
}

double DCMotor::getSpeed() {
    static bool initialized = false;
    static struct timeval timeVal;
    static struct timezone timeZone;
    static unsigned int i;
    static double times[4];
    static int32_t ticks[4];
    if (!initialized) { // the first time, populate these arrays
        for (int j=0; j<4; j++) {
            gettimeofday(&timeVal, &timeZone);
            times[j] =  timeVal.tv_sec + (timeVal.tv_usec/1000000.0);
            ticks[j] = decoder.readCntr();
            usleep(5000);
        }
        initialized = true;
    }
    speed_mutex.lock();
    gettimeofday(&timeVal, &timeZone);
    times[i] =  timeVal.tv_sec + (timeVal.tv_usec/1000000.0);
    ticks[i] = decoder.readCntr();

    degSpeed = ( (ticks[i] - ticks[(i+1)%4]) /
                 (times[i] - times[(i+1)%4]) ) * DEG_PER_CNT;

    if (++i > 3) i = 0;
    speed_mutex.unlock();

    return degSpeed;
}

// return the current position in degrees from home
double DCMotor::getPosition() {
    pos_mutex.lock();
    int count = decoder.readCntr();
    degPosition = (count % (int) CNT_PER_REV) * DEG_PER_CNT;
    if (count < 0) degPosition += 360;
    pos_mutex.unlock();
    return degPosition;
}

int32_t DCMotor::getCnt() {
    return decoder.readCntr();
}

void DCMotor::posPID() {
    printf("starting pid for position %f\n", setPos);
    PID pid(0.02, 1, 0.04, 0);
    //pid.setLimits(-30, 30);
    pid.setDampening(-0.08, 0.08);
    pid.setRollover(0, 360);
    pid.setDeadzone(-13, 13);
    double output;
    while (runningPID.load()) {
        pid.update(setPos, getPosition());
        output = pid.getOutput();
        setSpeed((int) output);
        //setSpeed((int) pid.getOutput());
        //printf("    setPoint: %-.4f, proccessValue: %-.4f, output: %-.4f\n", setPos, getPosition(), output);
        usleep(20000);
    }
}

void DCMotor::speedPID() {
    printf("starting pid for speed %f\n", setSpd);
    PID pid(0.02, 0.01, 0.004, 0);
    pid.setLimits(-20, 20);
    //pid.setDampening(-0.08, 0.08);
    double output;
    while (runningPID.load()) {
        pid.update(setSpd, getSpeed());
        output = pid.getOutput();
        setSpeed((int) (output + pwmSpeed));
        printf("pwmSpeed: %d    ", pwmSpeed);
        //setSpeed((int) pid.getOutput() + pwmSpeed);
        printf("setPoint: %-.4f, proccessValue: %-.4f, output: %-.4f    ", setSpd, getSpeed(), output);
        usleep(20000);
    }
}

void DCMotor::stopPID() {
    runningPID.store(false);
    usleep(50000);
}

void DCMotor::pidPosition(double position) {
    motor.stopPID();
    motor.setGradSpeed(0);
    setPos = position;
    runningPID.store(true);
    std::thread thr(&DCMotor::posPID, this);
    thr.detach();
}

void DCMotor::pidSpeed(double speed) {
    stopPID();
    setSpd = speed;
    runningPID.store(true);
    std::thread thr(&DCMotor::speedPID, this);
    thr.detach();
}
