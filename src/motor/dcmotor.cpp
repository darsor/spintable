#include "dcmotor.h"
#include "pid.h"
#include <wiringPi.h>
#include <sys/time.h>
#include <unistd.h>
#include <thread>
//#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <mutex>

std::mutex speed_mutex;
std::mutex pos_mutex;
std::atomic<bool> index_tripped;
void indexISR() { index_tripped.store(true); }

DCMotor::DCMotor(int channel, int addr, int freq) try : pwm(addr), decoder() {
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
    index_tripped.store(false);
    indexPin = 6;
    pwmSpeed = 0;
    pwmSpeedOld = 0;
    pwm.setPWMFreq(freq); // default @1600Hz PWM freq
    run(RELEASE);
    wiringPiSetup();
    if (wiringPiISR(indexPin, INT_EDGE_RISING, &indexISR) < 0) {
        perror("Unable to setup ISR");
    } else printf("Set up ISR\n");
} catch (...) {}

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
	if (speed < -4095) speed = -4095;
	if (speed > 4095) speed = 4095;
    if (speed == 0) {
        run(RELEASE);
    } else if (speed > 0) {
        run(BACKWARD);
        pwm.setPWM(pwmPin, 0, speed);
    } else if (speed < 0) {
        run(FORWARD);
        pwm.setPWM(pwmPin, 0, abs(speed));
    }
    pwmSpeed = speed;
}

void DCMotor::setGradSpeed(int speed) {
    stopPID();
    if (speed > 4095) {
        speed = 4095;
    } else if (speed < -4095) {
        speed = -4095;
    }
    //printf("speed changing from %d to %d\n", pwmSpeedOld, speed);

    if (speed < pwmSpeedOld) inc = -1;
    else inc = 1;

    for (int i=pwmSpeedOld; i != speed ; i += inc) {
        //printf("old speed: %d, current speed: %d, new speed: %d\n", pwmSpeedOld, i, speed);
        setSpeed(i);
        usleep(150);
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
    stopPID();
    decoder.clearCntr();
}

// make sure (0 <= deg < 360)
double degCoerce(double deg) {
    if (deg >= 360) deg -= 360;
    else if (deg < 0) deg += 360;
    return deg;
}

// take a degree and return the closest degree that matches a quadrature pulse
double degToCnt(double deg) {
    double diff = fmod(deg, 0.15);
    if (diff > 0.075) deg += 0.15-diff;
    else if (diff < 0.075) deg -= diff;
    return degCoerce(deg);
}

void DCMotor::gotoIndex() {
    stopPID();
    setGradSpeed(0);
    // index pulses are 75 degrees apart, initial increment is 75 degrees
    double inc = 75;
    int i=0;
    // record starting position
    double position = getPosition();
    pidPosition(position);
    // do a binary search for the index pulse
    while (true) {
        index_tripped.store(false);
        i = 0;
        while (!index_tripped.load()) {
            if (i >= 3) {
                inc *= -1;
                i = 0;
            }
            //printf("sweeping from %f to %f\n", setPos, degCoerce(setPos + inc/2));
            setPos = degCoerce(setPos + inc/2);
            while (abs(setPos - getPosition())>0.01875) // compare the two within a threshold (wait until we're at the position)
                usleep(100000);
            usleep(300000);
            //printf("stopped at %f\n", setPos);
            i++;
        }
        if (digitalRead(indexPin)) break;
        //printf("increment changing from %f to %f\n", inc, inc/-2.0);
        inc /= -2.0;
        if (fabs(inc) < 0.01) break;
    }
    //printf("gotoIndex stopping at %f\n", setPos);
}

// TODO: currently broken, needs to store times/ticks with packet cycle
double DCMotor::getSpeed() {
    static const unsigned int samples = 5;
    static bool initialized = false;
    static struct timeval timeVal;
    static unsigned int i;
    static double times[samples]; // timestamps of encoder counds
    static double diffs[samples]; // diferences between ticks
    static int32_t ticks[samples]; // encoder counts
    speed_mutex.lock();
    gettimeofday(&timeVal, nullptr);
    if (!initialized) { // the first time, populate these arrays
        for (unsigned int j=0; j<samples; j++) {
            gettimeofday(&timeVal, nullptr);
            times[j] =  timeVal.tv_sec + (timeVal.tv_usec/1000000.0);
            ticks[j] = currentCnt;
            diffs[j] = 0;
            usleep(5000);
        }
        initialized = true;
    }
    times[i] =  timeVal.tv_sec + (timeVal.tv_usec/1000000.0);
    ticks[i] = currentCnt;
    // simple low-pass filtered tick differences
    diffs[i] = 0.20 * (ticks[i] - ticks[(i+1)%samples]) + 0.80 * diffs[(i-1+samples)%samples];

    degSpeed = ( diffs[i] / (times[i] - times[(i+1)%samples]) ) * DEG_PER_CNT;

    if (++i > samples-1) i = 0;
    speed_mutex.unlock();

    return degSpeed;
}

// return the current position in degrees from home
double DCMotor::getPosition() {
    pos_mutex.lock();
    degPosition = (currentCnt % (int) CNT_PER_REV) * DEG_PER_CNT;
    if (currentCnt < 0) degPosition += 360;
    pos_mutex.unlock();
    return degPosition;
}

int32_t DCMotor::getCnt() {
    return currentCnt;
}

int32_t DCMotor::updateCnt() {
    currentCnt = decoder.readCntr();
    return currentCnt;
}

void DCMotor::posPID() {
    printf("starting pid for position %f\n", setPos);
    //PID pid(0.002, 11.5, 0.1, 0.08);
    PID pid(0.002, 14, 3, 6);
    pid.setDampening(-0.01875, 0.01875);
    pid.setRollover(0, 360);
    pid.setDeadzone(-250, 250);
    pid.setIntegralRange(-10, 10);
    pid.setLimits(-1200, 1200);
    double output;
    while (runningPID.load()) {
        pid.update(setPos, getPosition());
        output = pid.getOutput();
        setSpeed((int) output);
        //setSpeed((int) pid.getOutput());
        //printf("    setPoint: %-.4f, proccessValue: %-.4f, output: %-.4f\n", setPos, getPosition(), output);
        usleep(500);
    }
}

void DCMotor::speedPID() {
    printf("starting pid for speed %f\n", setSpd);
    PID pid(0.01, 0.2, 0, 0);
    //pid.setLimits(-10, 10);
    pid.setDeadzone(-1, 1);
    //pid.setDampening(-0.08, 0.08);
    double output;
    while (runningPID.load()) {
        pid.update(setSpd, getSpeed());
        output = pid.getOutput();
        setSpeed((int) (output + pwmSpeed));
        //printf("pwmSpeed: %d    ", pwmSpeed);
        //setSpeed((int) pid.getOutput() + pwmSpeed);
        //printf("setPoint: %-.4f, proccessValue: %-.4f, output: %-.4f    ", setSpd, getSpeed(), output);
        usleep(10000);
    }
}

void DCMotor::stopPID() {
    runningPID.store(false);
    usleep(50000);
}

void DCMotor::pidPosition(double position) {
    stopPID();
    setGradSpeed(0);
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
