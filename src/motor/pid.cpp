#include "pid.h"
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>

PID::PID(double dt, double Kp, double Ki, double Kd) :
    dt(dt), Kp(Kp), Ki(Ki), Kd(Kd) {
    outputLimits = false;
    dampening = false;
    rollover = false;
    deadzone = false;
    tuning = false;
    integral = 0;
    pError = 0;
}

void PID::update(double sp, double pv) {
    if (tuning && sp != setPointOld) {
        std::string str;
        std::cout << "\nTune constants? [y/n]: ";
        char r = getchar();
        getchar();
        if (r == 'y' || r == 'Y') {
            std::cout << "Enter new constants (<Kp> <Ki> <Kd>): ";
            std::cin >> str;
            Kp = atof(str.c_str());
            std::cin >> str;
            Ki = atof(str.c_str());
            std::cin >> str;
            Kd = atof(str.c_str());
        }
    }
    setPointOld = sp;
    error = sp-pv;
    if (rollover) {
        if (abs(error) > ((rollHigh - rollLow)/2)) {
            if (error > 0) error -= rollHigh;
            else error += rollHigh;
        }
    }
    if (dampening && error < dampHigh && error > dampLow) {
        integral *= 0.5;
        error = 0;
    }
    integral += error*dt;
    derivative = (error-pError)/dt;
    //printf("proportional: %-.4f, integral: %-.4f, derivative: %-.4f, error: %-.4f     \n", Kp*error, Ki*integral, Kd*derivative, error);
    output = Kp*error + Ki*integral + Kd*derivative;
    pError = error;

    if (outputLimits) {
        if (output > upper) output = upper;
        if (output < lower) output = lower;
    }
    if (deadzone && error != 0) {
        if (output < deadHigh && output > deadLow) {
            if (error > 0) {
                output = deadHigh + Ki*integral;
            } else if (error < 0) {
                output = deadLow - Ki*integral;
            }
        }
    }
}

void PID::changeConstants(double p, double i, double d) {
    Kp = p; Ki = i; Kd = d;
}

void PID::setLimits(double low, double high) {
    outputLimits = true;
    lower = low;
    upper = high;
}

void PID::setDampening(double low, double high) {
    dampening = true;
    dampLow = low;
    dampHigh = high;
}

void PID::setRollover(double low, double high) {
    rollover = true;
    rollLow = low;
    rollHigh = high;
}

void PID::setDeadzone(double low, double high) {
    deadzone = true;
    deadLow = low;
    deadHigh = high;
}
