#include "cosmos/cosmosQueue.h"
#include "gps/gps.h"
#include "motor/dcmotor.h"
#include <wiringPi.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>
#include <cmath>
#include <mutex>
#include <condition_variable>

// gather data from various sensors
void encoder(EncoderPacket* p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// set up condition variable to wake up the GPS thread
std::mutex gps_m;
std::condition_variable gps_cv;
bool gpsStart = false;

// initialize GPS and its packet
Gps gps(2, "/dev/ttyAMA0", 9600);
TimePacket* tPacket = NULL;

// initialize COSMOS and devices
// these are global so that all threads can access them
CosmosQueue queue(128);
DCMotor motor(2, 0x60, 1600);

PI_THREAD (motorControl) {
    static char buffer[6];
    static uint16_t length;
    static uint16_t id;
    static int16_t speed;
    static float pos;
    static double position;
    while (true) {
        while (!queue.isConnected()) usleep(100000); // if it's disconnected, loop and wait until it reconnects
        // receive the first two bytes (the length of the packet);
        if (queue.recv(buffer, 2) != 0) {
            // if connection was lost, sending packets will fail and then try to reconnect
            motor.stopPID();
            motor.setSpeed(0);
            usleep(1000000);
            continue;
        } else {
            length = ntohs(*((uint16_t*)buffer));
            // receive the rest of the packet
            if (queue.recv(buffer, length) != 0) {
                motor.stopPID();
                motor.setSpeed(0);
                continue;
            }
            // get the id to know what command it is
            id = ntohs(*((uint16_t*)buffer));
            switch (id) {
                case 1: // set home command
                    motor.setHome();
                    break;
                case 2: // change speed command
                    speed = ntohs(*((int16_t*)(buffer+2)));
                    printf("received command to change speed to %d\n", speed);
                    motor.setGradSpeed(speed);
                    break;
                case 3: // go to absolute position command
                    pos = *( (float*) (buffer+2));
                    endianSwap(pos);
                    printf("received command to change absolute position to %f\n", pos);
                    motor.pidPosition(pos);
                    break;
                case 4: // go to relative position command
                    pos = *( (float*) (buffer+2));
                    endianSwap(pos);
                    printf("received command to change relative position by %f\n", pos);
                    if (motor.pidIsOn()) position = motor.getPidPos();
                    else position = motor.getPosition();
                    position += pos;
                    if (position < 360) position += 360;
                    if (position > 360) position -= 360;
                    motor.pidPosition(position);
                    break;
                case 5: // go to index command (not implemented!)
                    printf("received command to gotoIndex\n");
                    motor.gotoIndex();
                    break;
                default:
                    printf("unknown command received\n");
                    usleep(500000);
            }
        }
        id = 0;
    }
}

PI_THREAD (gpsControl) {
    TimePacket* tempPacket;
    while (true) {
        std::unique_lock<std::mutex> lk(gps_m);
        gps_cv.wait(lk, []{return gpsStart;});
        tempPacket = tPacket;
        tempPacket->gpsTime = gps.getTime();
        gpsStart = false;
        lk.unlock();
        queue.push(tempPacket);
    }
}

PI_THREAD (cosmosQueue) {
    while (true) {
        queue.connect();
        while (queue.isConnected()) {
            while (queue.pop());
            usleep(10000);
        }
        printf("connection with COSMOS lost\n");
    }
}

int main() {

    // set up wiringPi
    wiringPiSetup();

    // initialize encoder and its packet
    EncoderPacket* ePacket = NULL;
    encoder(ePacket);

    // start threads
    if (piThreadCreate(cosmosQueue) != 0) {
        perror("COSMOS queue thread didn't start");
    }
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
    }
    if (piThreadCreate(gpsControl) != 0) {
        perror("GPS control thread didn't start");
    }

    // set high priority for this thread
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0)
        system(cmd.str().c_str());

    // these values help time the packets
    long timer = 0, difference = 0;
    struct timeval start, next;
    while (true) {

        // get timestamps and send time packet
        tPacket = new TimePacket();
        gps.timestampPPS(tPacket->sysTimeSeconds, tPacket->sysTimeuSeconds);
        { // wait in another thread for the GPS to return its timestamp
            std::lock_guard<std::mutex> lk(gps_m);
            gpsStart = true;
        } gps_cv.notify_one();

        // set time values for timing
        start.tv_sec = tPacket->sysTimeSeconds;
        start.tv_usec = tPacket->sysTimeuSeconds;
        difference = 0;
        timer = 0;
        // every second, do this 50 times
        for (int j=0; j<50; j++) {

            do { // delay a bit (20ms per packet)
                gettimeofday(&next, NULL);
                if (next.tv_sec > start.tv_sec) {
                    difference = (1000000-start.tv_usec) + next.tv_usec;
                } else {
                    difference = next.tv_usec - start.tv_usec;
                }
                if (difference < timer) usleep(100);
            } while (difference < timer);
            if (difference > 982000) break;
            //printf("started cycle at %li/%li\n", difference, timer);

            ePacket = new EncoderPacket();
            systemTimestamp(ePacket->sysTimeSeconds, ePacket->sysTimeuSeconds);
            encoder(ePacket);
            queue.push(ePacket);

            timer += 20000;
        }
    }
    return 0;
}

void encoder(EncoderPacket* p) {
    static const unsigned int CNT_PER_REV = 2400;
    if (p == NULL) return;
    p->motorSpeed = motor.getSpeed();
    p->position = motor.getPosition();
    p->raw_cnt = motor.getCnt();
    p->rev_cnt = p->raw_cnt / CNT_PER_REV;
}

void systemTimestamp(uint32_t &stime, uint32_t &ustime) {
    static struct timeval timeVal;
    gettimeofday(&timeVal, NULL);
    stime =  timeVal.tv_sec;
    ustime = timeVal.tv_usec;
}
