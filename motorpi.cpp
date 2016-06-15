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
Gps gps(2, "/dev/ttyS0", 9600);
TimePacket* tPacket = nullptr;

// initialize COSMOS and devices
// these are global so that all threads can access them
CosmosQueue queue(4810, 128, 8);
DCMotor motor(2, 0x60, 1600);

PI_THREAD (motorControl) {
    Packet* cmdPacket = nullptr;
    double position = 0;
    while (true) {
        if (queue.pop_cmd(cmdPacket)) {
            switch (cmdPacket->id) {
                case MOTOR_SET_HOME_ID:
                    printf("received command to set encoder home\n");
                    motor.setHome();
                    break;
                case MOTOR_SET_SPEED_ID:
                    {
                        SetSpeedCmd* speedCmd = static_cast<SetSpeedCmd*>(cmdPacket);
                        speedCmd->SetSpeedCmd::convert();
                        printf("received command to change speed to %d\n", speedCmd->speed);
                        motor.setGradSpeed(speedCmd->speed);
                    }
                    break;
                case MOTOR_ABS_POS_ID:
                    {
                        SetAbsPosCmd* absCmd = static_cast<SetAbsPosCmd*>(cmdPacket);
                        absCmd->SetAbsPosCmd::convert();
                        printf("received command to change absolute position to %f\n", absCmd->position);
                        motor.pidPosition(absCmd->position);
                    }
                    break;
                case MOTOR_REV_POS_ID:
                    {
                        SetRevPosCmd* revCmd = static_cast<SetRevPosCmd*>(cmdPacket);
                        revCmd->SetRevPosCmd::convert();
                        printf("received command to change relative position by %f\n", revCmd->position);
                        if (motor.pidIsOn()) position = motor.getPidPos();
                        else position = motor.getPosition();
                        position += revCmd->position;
                        if (position < 360) position += 360;
                        if (position > 360) position -= 360;
                        motor.pidPosition(position);
                    }
                    break;
                case MOTOR_GOTO_INDEX_ID:
                    printf("received command to gotoIndex\n");
                    motor.gotoIndex();
                    break;
                default:
                    printf("unknown command received\n");
            }
            delete cmdPacket;
            cmdPacket = nullptr;
        }
        if (!queue.isConnected()) {
            motor.stopPID();
            motor.setSpeed(0);
        }
        usleep(200000);
    }
}

int main() {

    // initialize encoder and its packet
    EncoderPacket* ePacket = nullptr;
    encoder(ePacket);

    // start threads
    if (piThreadCreate(motorControl) != 0) {
        perror("Motor control thread didn't start");
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
        start.tv_sec = tPacket->sysTimeSeconds;
        start.tv_usec = tPacket->sysTimeuSeconds;
        tPacket->gpsTime = gps.getTime();
        queue.push_tlm(tPacket);
        difference = 0;
        timer = 0;

        // every second, do this 200 times
        for (int j=0; j<200; j++) {

            // synchronize the packets
            do {
                gettimeofday(&next, nullptr);
                difference = next.tv_usec - start.tv_usec + (next.tv_sec - start.tv_sec) * 1000000;
                if (difference < timer) usleep(100);
            } while (difference < timer);
            if (difference > 996000) break;

            //printf("started cycle at %li/%li\n", difference, timer);

            ePacket = new EncoderPacket();
            systemTimestamp(ePacket->sysTimeSeconds, ePacket->sysTimeuSeconds);
            encoder(ePacket);
            queue.push_tlm(ePacket);

            timer += 5000;
        }
    }
    return 0;
}

void encoder(EncoderPacket* p) {
    static const int CNT_PER_REV = 2400;
    if (p == nullptr) return;
    p->motorSpeed = motor.getSpeed();
    p->position = motor.getPosition();
    p->raw_cnt = motor.getCnt();
    p->rev_cnt = p->raw_cnt / CNT_PER_REV;
}

void systemTimestamp(uint32_t &stime, uint32_t &ustime) {
    static struct timeval timeVal;
    gettimeofday(&timeVal, nullptr);
    stime =  timeVal.tv_sec;
    ustime = timeVal.tv_usec;
}
