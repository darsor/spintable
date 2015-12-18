# import to control pins and interrupts
import time
import atexit
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

M_NUM = 3   # motor number (1-4)
PIN_A = 19  # encoder pins
PIN_B = 20
PIN_I = 21

# pull up both pins and get ready to read them
GPIO.setup(PIN_A, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(PIN_B, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(PIN_I, GPIO.IN, pull_up_down=GPIO.PUD_UP)

pos = 0
a_old = GPIO.input(PIN_A)
b_old = GPIO.input(PIN_B)
clockwise = True
inc = 0
inc_old = 0
time_new = 0
time_old = 0
speed = 0

# automatically clean up GPIO on exit
def cleanup():
    GPIO.cleanup()
atexit.register(cleanup)

def pulse(pin):
    global PIN_A
    global PIN_B
    global pos
    global a
    global b
    global a_old
    global b_old
    global clockwise

    a,b = GPIO.input(PIN_A),GPIO.input(PIN_B)
    
    if ((a,b_old) == (1,0)) or ((a,b_old) == (0,1)):
        pos += 1
        clockwise = True
    elif ((a,b_old) == (1,1)) or ((a,b_old) == (0,0)):
        pos -= 1
        clockwise = False

    a_old,b_old = a,b

def increment(pin):
    global inc
    global clockwise
    global time_new
    global time_old
    global speed
    time_new = time.time()
    speed = 1 / (4.8 * (time_new - time_old))
    time_old = time_new
    if clockwise:
        inc += 1
    else:
        inc -= 1

# set up interrupts
GPIO.add_event_detect(PIN_A, GPIO.BOTH, callback=pulse)
GPIO.add_event_detect(PIN_B, GPIO.BOTH, callback=pulse)
GPIO.add_event_detect(PIN_I, GPIO.RISING, callback=increment)


while (1):
    if (inc != inc_old):
        inc_old = inc
        print "position: {0:<10} increment: {1:<8} clockwise: {2:<5} speed: {3}".format(pos, inc, clockwise, speed)
    time.sleep(.05)
