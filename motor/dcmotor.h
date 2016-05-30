#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "pwm.h"
#include "decoder.h"
#include <unistd.h>
#include <atomic>

#define FORWARD     1
#define BACKWARD    2
#define BRAKE       3
#define RELEASE     4

class DCMotor {
    public:
        DCMotor(int channel=2, int addr=0x60, int freq=1600);
        ~DCMotor();
                        // TODO: mutex functions

        void run(int command);
        void setSpeed(int speed);
        void setGradSpeed(int speed);
        void setPin(int pin, int value);
        void setHome(); // set current position as home
        void gotoIndex(); // go to nearest index

        int getPwmSpeed() {return pwmSpeed;}
        double getSpeed(); // return speed in degrees/sec
        double getPosition(); // return position in degrees from home
        int32_t getCnt();

        void stopPID();
        void pidPosition(double setPosition);
        void pidSpeed(double speed);
        bool pidIsOn() { return runningPID.load(); }
        double getPidPos() { return setPos; }


    private:
        void posPID();
        void speedPID();
        double setPos;
        double setSpd;
        std::atomic<bool> runningPID;

        const unsigned int CNT_PER_INDEX = 500;
        const unsigned int CNT_PER_REV = 2400;
        const double DEG_PER_CNT = 360.0/CNT_PER_REV;

        int indexPin;
        int pwmPin;
        int in1Pin;
        int in2Pin;
        int i2cAddr;            // default addr on HAT is 0x60
        int freq;               // default @1600Hz PWM freq
        PWM pwm;
        Decoder decoder;

        int inc;
        int pwmSpeed;
        int pwmSpeedOld;
        double degSpeed;
        double degPosition;
};

#endif
