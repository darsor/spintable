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
CosmosQueue queue(4810, 600000, 8);

// the motor is global because the encoder is accessed through it.
// one process handles mtor commands, another reads the encoder
DCMotor* motor = nullptr;
int8_t skip_flag = 0;

// function prototype for getTimestamp() (defined at the bottom), which
// returns the time in microseconds since unix epoch
uint64_t getTimestamp();

// this thread connects to the motor/encoder and puts together an
// encoder packet containing 100 samples
void encoder_thread() {
    EncoderPacket* ePacket = nullptr;

    while (true) {
        // try opening the motor every 10 seconds until it succeeds
        while (true) {
            try {
                motor = new DCMotor(2, 0x60, 500);
                printf("Successfully connected to motor and quadrature decoder\n");
                sleep(1);
                break;
            } catch (int e) {
                motor = nullptr;
                printf("FAILED to connect to motor or quadrature decoder. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        try {
            int32_t last_cnt = motor->updateCnt();
            while (true) {
                ePacket = new EncoderPacket;

                for (int i=0; i<100; ++i) {
                    ePacket->timestamps[i] = getTimestamp();
                    ePacket->raw_cnts[i] = motor->updateCnt();
                    if (i == 0) {
                        if (abs(ePacket->raw_cnts[0] - last_cnt) > 1) {
                            skip_flag = 1;
                        }
                    } else if (abs(ePacket->raw_cnts[i] - ePacket->raw_cnts[i-1]) > 1) {
                        skip_flag = 1;
                    }
                    if (i == 99) {
                        last_cnt = ePacket->raw_cnts[i];
                    }
                    ePacket->skip_flag = skip_flag;
                }

                queue.push_tlm(ePacket);
            }
        } catch (int e) {
            printf("lost connection with motor and quadrature decoder\n");
            if (ePacket) delete ePacket;
            delete motor;
            motor = nullptr;
            continue;
        }
    }
}

// this thread handles commands to the motor
void motor_thread() {
    Packet* cmdPacket = nullptr;
    uint8_t id;
    double position = 0;
    while (true) {
        if (queue.cmdSize() > 0 && motor) {
            id = queue.cmd_front_id();
            position = 0;
            cmdPacket = nullptr;
            switch (id) {
                case MOTOR_SET_HOME_ID:
                    queue.pop_cmd(cmdPacket);
                    printf("received command to set encoder home\n");
                    skip_flag = 0;
                    motor->setHome();
                    break;
                case MOTOR_SET_SPEED_ID:
                    {
                        queue.pop_cmd(cmdPacket);
                        SetSpeedCmd* speedCmd = static_cast<SetSpeedCmd*>(cmdPacket);
                        speedCmd->SetSpeedCmd::convert();
                        printf("received command to change speed to %d\n", speedCmd->speed);
                        skip_flag = 0;
                        motor->setGradSpeed(speedCmd->speed);
                    }
                    break;
                case MOTOR_ABS_POS_ID:
                    {
                        queue.pop_cmd(cmdPacket);
                        SetAbsPosCmd* absCmd = static_cast<SetAbsPosCmd*>(cmdPacket);
                        absCmd->SetAbsPosCmd::convert();
                        printf("received command to change absolute position to %f\n", absCmd->position);
                        skip_flag = 0;
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
                        skip_flag = 0;
                        motor->pidPosition(position);
                    }
                    break;
                case MOTOR_GOTO_INDEX_ID:
                    queue.pop_cmd(cmdPacket);
                    printf("received command to gotoIndex\n");
                    skip_flag = 0;
                    motor->gotoIndex();
                    break;
                default:
                    printf("unknown command received with id=%d\n", id);
            }
            if (cmdPacket) delete cmdPacket;
        } else {
            usleep(200000);
        }
        if (!queue.isConnected() && motor) {
            motor->stopPID();
            motor->setSpeed(0);
        }
    }
}

// this thread puts together a housekeeping packet that it
// queues every second
void housekeeping_thread() {

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

    // set high priority for the encoder
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());

    // launch the instrument threads
    std::thread(encoder_thread).detach();

    cmd.str("");
    cmd << "renice -n +1 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());
    std::thread(housekeeping_thread).detach();

    cmd.str("");
    cmd << "renice -n 0 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());
    std::thread(motor_thread).detach();

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
        while (!gps->dataAvail()) usleep(10000);
        try {
            while (true) {
                tPacket = new TimePacket;

                gps->timestampPPS(tPacket->systemTime);
                if (tPacket->systemTime == 0) {
                    delete tPacket;
                    continue;
                }
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
