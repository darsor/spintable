#!/usr/bin/python
from Adafruit_MotorHAT import Adafruit_MotorHAT, Adafruit_DCMotor
#from Adafruit_PWM_Servo_Driver import PWM
import sys

import time
import atexit

#pwm = PWM(0x40)
#pwm.setPWMFreq(60)

# create a default object, no changes to I2C address or frequency
mh = Adafruit_MotorHAT(addr=0x60)

# recommended for auto-disabling motors on shutdown!
def turnOffMotors():
	mh.getMotor(1).run(Adafruit_MotorHAT.RELEASE)
	mh.getMotor(2).run(Adafruit_MotorHAT.RELEASE)
	mh.getMotor(3).run(Adafruit_MotorHAT.RELEASE)
	mh.getMotor(4).run(Adafruit_MotorHAT.RELEASE)

atexit.register(turnOffMotors)

################################# DC motor test!
myMotor = mh.getMotor(1)

speed = 0
speed_old = 0
inc = 0

while (True):
    try:
        speed = input("Input speed (from -255 to 255): ")
    except SyntaxError: # if invalid/nothing entered, exit program
        turnOffMotors()
        sys.exit()

    # set an increment for gradual speed change
    if speed > speed_old:
        inc = 1
    else:
        inc = -1

    # gradual speed up/down
    for i in range(speed_old, speed, inc):
        if i < 0:
            myMotor.run(Adafruit_MotorHAT.BACKWARD)
        else:
            myMotor.run(Adafruit_MotorHAT.FORWARD)

        myMotor.setSpeed(abs(i))
        time.sleep(0.01)

    speed_old = speed

turnOffMotors()
