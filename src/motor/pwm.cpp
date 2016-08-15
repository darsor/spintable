#include "pwm.h"
#include <unistd.h>
#include <cstdlib>
#include <cstdint>
#include <cmath>

PWM::PWM(int address=0x40) {
    pwm = wiringPiI2CSetup (address);
    if (pwm < 0) {
        throw 1;
    }
    wiringPiI2CWriteReg8(pwm, MODE2, OUTDRV);
    wiringPiI2CWriteReg8(pwm, MODE1, ALLCALL);
    usleep(5000); // wait for oscillator
    uint8_t mode1 = wiringPiI2CReadReg8 (pwm, MODE1);
    mode1 = mode1 & ~SLEEP;
    wiringPiI2CWriteReg8(pwm, MODE1, mode1);
    usleep(5000); // wait for oscillator
}

PWM::~PWM() {
    close(pwm);
}

void PWM::setPWMFreq(int freq) {
    float prescaleval = 25000000.0; // 25MHz
    prescaleval /= 4096.0;          // 12-bit
    prescaleval /= (float) freq;
    prescaleval -= 1.0;
    float prescale = floor(prescaleval + 0.5);

    uint8_t oldmode = wiringPiI2CReadReg8(pwm, MODE1);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;          // sleep
    wiringPiI2CWriteReg8(pwm, MODE1, newmode);  // go to sleep
    wiringPiI2CWriteReg8(pwm, PRESCALE, (int) floor(prescale));
    wiringPiI2CWriteReg8(pwm, MODE1, oldmode);
    usleep(5000);
    wiringPiI2CWriteReg8(pwm, MODE1, oldmode | 0x80);
}

void PWM::setPWM(int channel, int on, int off) {
    wiringPiI2CWriteReg8(pwm, LED_ON_L+4*channel, on & 0xFF);
    wiringPiI2CWriteReg8(pwm, LED_ON_H+4*channel, on >> 8);
    wiringPiI2CWriteReg8(pwm, LED_OFF_L+4*channel, off & 0xFF);
    wiringPiI2CWriteReg8(pwm, LED_OFF_H+4*channel, off >> 8);
}
