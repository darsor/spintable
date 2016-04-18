#include "pid.h"
#include <cstdio>
#include <cstdlib>

PID::PID(double dt, double Kp, double Ki, double Kd) :
    dt(dt), Kp(Kp), Ki(Ki), Kd(Kd) {
    outputLimits = false;
    dampening = false;
    rollover = false;
    deadzone = false;
    integral = 0;
    pError = 0;
}

void PID::update(double sp, double pv) {
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
    //printf("proportional: %-.4f, integral: %-.4f, derivative: %-.4f, error: %-.4f     ", Kp*error, Ki*integral, Kd*derivative, error);
    output = Kp*error + Ki*integral + Kd*derivative;
    pError = error;

    if (outputLimits) {
        if (output > upper) output = upper;
        if (output < lower) output = lower;
    }
    if (deadzone && error != 0) {
        if (output < deadHigh && output > deadLow) {
            if (error > 0) {
                output = deadHigh;
            } else if (error < 0) {
                output = deadLow;
            }
        }
    }
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
