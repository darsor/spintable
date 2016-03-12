# import to control pins and interrupts
import time
import atexit
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)

PIN_A = 19          # encoder pins
PIN_B = 20
PIN_I = 21
clockwise = True    # direction of spin
res = 8            # number of index pulse samples used to find the speed (higher = more accurate)
inc = 0             # index pulse counter
inc_old = 0
speed = 0           # speed in rotations/second
times = [0] * res   # list to hold 'res' time samples
times_cnt = 0

# pull up input pins and get ready to read them
GPIO.setup(PIN_A, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(PIN_B, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(PIN_I, GPIO.IN, pull_up_down=GPIO.PUD_UP)

a = GPIO.input(PIN_A)
b = GPIO.input(PIN_B)

# automatically clean up GPIO on exit
def cleanup():
    GPIO.cleanup()
atexit.register(cleanup)

# ISR's for each encoder pin
def pulseA(pin):
    global a
    global b
    global clockwise
    global PIN_A
    a = GPIO.input(PIN_A)
    if a == 1 and b == 1:
        clockwise = True

def pulseB(pin):
    global a
    global b
    global clockwise
    global PIN_B
    b = GPIO.input(PIN_B)
    if a == 1 and b == 1:
        clockwise = False

def increment(pin):
    global inc
    global clockwise
    global speed
    global times
    global times_cnt
    global res
    times[times_cnt] = time.time()
    speed = res / (4.8 * (times[times_cnt] - times[(times_cnt+1)%res]))
    times_cnt += 1
    if times_cnt > res-1:
        times_cnt = 0
    if clockwise:
        inc += 1
    else:
        inc -= 1

# set up interrupts
GPIO.add_event_detect(PIN_A, GPIO.BOTH, callback=pulseA)
GPIO.add_event_detect(PIN_B, GPIO.BOTH, callback=pulseB)
GPIO.add_event_detect(PIN_I, GPIO.RISING, callback=increment)

# dump encoder data
while (1):
    if (inc != inc_old):
        inc_old = inc
        print "increment: {0:<8} clockwise: {1:<5} speed: {2:.4f}".format(inc, clockwise, speed)
    time.sleep(.05)
