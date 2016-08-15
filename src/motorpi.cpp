#include "cosmos/cosmosQueue.h" // for cosmos and packet queues
#include "gps/gps.h"            // for gps interface
#include "motor/dcmotor.h"      // for motor interface
#include <sys/sysinfo.h>        // for memory usage info
#include <fstream>              // for reading files (housekeeping)
#include <sstream>              // for sstream objects
#include <cstdio>               // for printf() and perror()
#include <thread>               // for multithreading
#include <chrono>               // for timestamps

// this queue automatically opens a port for COSMOS.
// any packets pushed to the tlm_queue will be sent to COSMOS.
// any received packets will be found on the cmd_queue.
// it is global so that all threads can access it
//TODO: calculate max size of queue
CosmosQueue queue(4810, 120000, 8);

// function prototype for getTimestamp() (defined at the bottom), which
// returns the time in microseconds since unix epoch
uint64_t getTimestamp();

// this thread connects to the motor/encoder and puts together an
// encoder packet that it queues every second.
// it also takes commands for the motor
void motor_thread() {
    EncoderPacket* ePacket = nullptr;
    DCMotor* motor = nullptr;

    while (true) {
        // try opening the motor every 10 seconds until it succeeds
        while (true) {
            try {
                motor = new DCMotor(2, 0x60, 500);
                printf("Successfully connected to motor and quadrature decoder\n");
                sleep(1);
                break;
            } catch (int e) {
                printf("FAILED to connect to motor or quadrature decoder. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        try {
            while (true) {
                ePacket = new EncoderPacket;

                ePacket->timestamp = getTimestamp();
                ePacket->raw_cnt = motor->updateCnt();

                queue.push_tlm(ePacket);

                if (queue.cmdSize() > 0) {
                    uint8_t id = queue.cmd_front_id();
                    double position = 0;
                    Packet* cmdPacket = nullptr;
                    switch (id) {
                        case MOTOR_SET_HOME_ID:
                            queue.pop_cmd(cmdPacket);
                            printf("received command to set encoder home\n");
                            motor->setHome();
                            break;
                        case MOTOR_SET_SPEED_ID:
                            {
                                queue.pop_cmd(cmdPacket);
                                SetSpeedCmd* speedCmd = static_cast<SetSpeedCmd*>(cmdPacket);
                                speedCmd->SetSpeedCmd::convert();
                                printf("received command to change speed to %d\n", speedCmd->speed);
                                motor->setGradSpeed(speedCmd->speed);
                            }
                            break;
                        case MOTOR_ABS_POS_ID:
                            {
                                queue.pop_cmd(cmdPacket);
                                SetAbsPosCmd* absCmd = static_cast<SetAbsPosCmd*>(cmdPacket);
                                absCmd->SetAbsPosCmd::convert();
                                printf("received command to change absolute position to %f\n", absCmd->position);
                                motor->pidPosition(absCmd->position);
                            }
                            break;
                        case MOTOR_REV_POS_ID:
                            {
                                queue.pop_cmd(cmdPacket);
                                SetRevPosCmd* revCmd = static_cast<SetRevPosCmd*>(cmdPacket);
                                revCmd->SetRevPosCmd::convert();
                                printf("received command to change relative position by %f\n", revCmd->position);
                                if (motor->pidIsOn()) position = motor->getPidPos();
                                else position = motor->getPosition();
                                position += revCmd->position;
                                if (position < 360) position += 360;
                                if (position > 360) position -= 360;
                                motor->pidPosition(position);
                            }
                            break;
                        case MOTOR_GOTO_INDEX_ID:
                            queue.pop_cmd(cmdPacket);
                            printf("received command to gotoIndex\n");
                            motor->gotoIndex();
                            break;
                        default:
                            printf("unknown command received with id=%d\n", id);
                    }
                    if (cmdPacket) delete cmdPacket;
                }
                if (!queue.isConnected()) {
                    motor->stopPID();
                    motor->setSpeed(0);
                }
            }
        } catch (int e) {
            printf("lost connection with motor and quadrature decoder\n");
            if (ePacket) delete ePacket;
            delete motor;
            continue;
        }
    }
}

// this thread puts together a housekeeping packet that it
// queues every second
void housekeeping_thread() {

    // set low priority for this thread
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n +1 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());

    // load needed files
    HKPacket* hkPacket = nullptr;
    std::ifstream tempfile("/sys/class/thermal/thermal_zone0/temp");
    std::ifstream loadfile("/proc/loadavg");
    std::string tmp;
    struct sysinfo memInfo;

    // get and send housekeeping packet repeatedly
    while (true) {
        hkPacket = new HKPacket;
        hkPacket->timestamp = getTimestamp();
        hkPacket->queue_size = queue.tlmSize();

        // look at the system file that has the cpu temperature
        tempfile.seekg(std::ios_base::beg);
        getline(tempfile, tmp);
        hkPacket->cpu_temp = stof(tmp) / 1000;

        // look at the system file that has the cpu load average
        loadfile.seekg(std::ios_base::beg);
        getline(loadfile, tmp);
        hkPacket->cpu_load = stof(tmp);

        // get currently used virtual memory
        sysinfo(&memInfo);
        hkPacket->mem_usage = memInfo.totalram - memInfo.freeram;
        hkPacket->mem_usage += memInfo.totalswap - memInfo.freeswap;
        hkPacket->mem_usage *= memInfo.mem_unit;

        queue.push_tlm(hkPacket);
        sleep(1);
    }
    tempfile.close();
    loadfile.close();
}

// main opens the GPS and sends time packets
int main() {

    // launch the instrument threads
    std::thread(motor_thread).detach();
    std::thread(housekeeping_thread).detach();

    // the main process gathers and sends GPS packets
    TimePacket* tPacket = nullptr;
    Gps* gps = nullptr;

    while (true) {
        // try opening the GPS every ten seconds until it succeeds
        while (true) {
            try {
                gps = new Gps(2, "/dev/ttyS0", 9600);
                printf("Successfully connected to the GPS\n");
                break;
            } catch (int e) {
                printf("FAILED to connect to the GPS. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        // make sure data is available before we get the PPS (avoids blocking on the gps->getTime() call)
        while(!gps->dataAvail()) usleep(10000);
        try {
            while (true) {
                tPacket = new TimePacket;

                gps->timestampPPS(tPacket->systemTime);
                tPacket->gpsTime = gps->getTime();

                queue.push_tlm(tPacket);
            }
        } catch (int e) {
            printf("lost connection with GPS\n");
            if (tPacket) delete tPacket;
            delete gps;
            continue;
        }
    }

    return 0;
}

// return the system time in microseconds since unix epoch
uint64_t getTimestamp() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
