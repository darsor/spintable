#include "pid.h"
#include <cstdio>

PID::PID(double dt, double Kp, double Ki, double Kd) :
    dt(dt), Kp(Kp), Ki(Ki), Kd(Kd) {
    outputLimits = false;
    dampening = false;
    integral = 0;
    pError = 0;
}

void PID::update(double sp, double pv) {
    error = sp-pv;
    if (dampening && error < dampHigh && error > dampLow) {
        printf("dampened\n");
        integral *= 0.5;
    }
    integral += error*dt;
    derivative = (error-pError)/dt;
    printf("proportional: %-.4f, integral: %-.4f, derivative: %-.4f\n", Kp*error, Ki*integral, Kd*derivative);
    output = Kp*error + Ki*integral + Kd*derivative;
    if (outputLimits) {
        if (output > upper) output = upper;
        if (output < lower) output = lower;
    }
    pError = error;
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
