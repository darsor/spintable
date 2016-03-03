#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "pwm.h"

#define FORWARD     1
#define BACKWARD    2
#define BRAKE       3
#define RELEASE     4

class DCMotor {
    public:
        DCMotor(int channel, int addr, int freq);
        ~DCMotor();

        void run(int command);
        void setSpeed(int speed);
        void setGradSpeed(int speed);
        void setPin(int pin, int value);

        int getSpeed() {return mSpeed;}

    private:
        int pwmPin;
        int in1Pin;
        int in2Pin;
        int i2cAddr;            // default addr on HAT is 0x60
        int freq;               // default @1600Hz PWM freq
        PWM pwm;

        int inc;
        int mSpeed;
        int mSpeedOld;
};

#endif
