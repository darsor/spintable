#ifndef DCMOTOR_H
#define DCMOTOR_H

#include <pwm.h>

#define FORWARD     1
#define BACKWARD    2
#define BRAKE       3
#define RELEASE     4

class DCMotor {
    public:
        int pwmPin;
        int in1Pin;
        int in2Pin;
        int i2cAddr;            // default addr on HAT
        int freq;               // default @1600Hz PWM freq

        DCMotor(int addr, int freq);

        void run(int command);
        void setSpeed(int speed);
        void setPin(int pin, int value);
    private:
        PWM pwm;
};

#endif
