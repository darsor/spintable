#ifndef PWM_H
#define PWM_H
#include <wiringPiI2C.h>

// ============================================================================
// Adafruit PCA9685 16-Channel PWM Servo Driver
// Ported from Adafruit-Motor-HAT-Python-Library into c++
// ============================================================================

// Registers/etc.
#define MODE1                0x00
#define MODE2                0x01
#define SUBADR1              0x02
#define SUBADR2              0x03
#define SUBADR3              0x04
#define PRESCALE             0xFE
#define LED_ON_L             0x06
#define LED_ON_H             0x07
#define LED_OFF_L            0x08
#define LED_OFF_H            0x09
#define ALL_LED_ON_L         0xFA
#define ALL_LED_ON_H         0xFB
#define ALL_LED_OFF_L        0xFC
#define ALL_LED_OFF_H        0xFD

// Bits
#define RESTART              0x80
#define SLEEP                0x10
#define ALLCALL              0x01
#define INVRT                0x10
#define OUTDRV               0x04

class PWM {
    public:
        PWM(int address); // defaults to 0x40
        ~PWM();
        void setPWMFreq(int freq);
        void setPWM(int channel, int on, int off);
    private:
        int pwm;
};

#endif
