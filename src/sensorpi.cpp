#include "cosmos/cosmosQueue.h" // for cosmos and packet queues
#include "gps/gps.h"            // for gps interface
#include "tam/tam.h"            // for tam interface
#include "imu/imu.h"            // for imu interface
#include <raspicam/raspicam.h>  // for camera interface
#include <sys/sysinfo.h>        // for memory usage info
#include <chrono>               // for timestamps
#include <thread>               // for multithreading
#include <unistd.h>             // for linux functions
#include <sstream>              // for sstream objects
#include <fstream>              // for reading files (housekeeping)
#include <cstdio>               // for perror() and printf()

// this queue automatically opens a port for COSMOS.
// any packets pushed to the tlm_queue will be sent to COSMOS.
// any received packets will be found on the cmd_queue.
// it is global so that all threads can access it
CosmosQueue queue(4810, 20000, 8);

// function prototype for getTimestamp() (defined at the bottom), which
// returns the time in microseconds since unix epoch
uint64_t getTimestamp();

// this thread creates and sends TamPackets
void tam_thread() {

    TamPacket* tPacket = nullptr;
    Tam* tam = nullptr;

    // try opening the TAM every ten seconds until it succeeds
    while (true) {
        try {
            tam = new Tam(100, 0x48);
            printf("Successfully connected to the TAM\n");
            break;
        } catch (int e) {
            printf("FAILED to connect to the TAM. Trying again in 10 seconds...\n");
            sleep(10);
        }
    }

    // repeatedly create and send packets
    while(true) {
        tPacket = new TamPacket;

        tPacket->timeA = getTimestamp();
        tPacket->tamA = tam->getData(0);
        tPacket->timeB = getTimestamp();
        tPacket->tamB = tam->getData(1);
        tPacket->timeC = getTimestamp();
        tPacket->tamC = tam->getData(2);

        queue.push_tlm(tPacket);
    }
}

// this thread creates and sends ImuPackets
void imu_thread() {

    ImuPacket* iPacket = nullptr;
    Imu* imu = nullptr;

    while (true) {
        // try opening the IMU every ten seconds until it succeeds
        while (true) {
            try {
                imu = new Imu();
                printf("Successfully connected to the IMU\n");
                sleep(2);
                break;
            } catch (int e) {
                printf("FAILED to connect to the IMU. Trying again in 10 seconds...\n");
                sleep(10);
            }
        }

        try {
            while (true) {
                iPacket = new ImuPacket;

                iPacket->dataTimestamp = getTimestamp();
                imu->getdata(iPacket->imuData);
                iPacket->quatTimestamp = getTimestamp();
                imu->getQuaternion(iPacket->imuQuat);

                queue.push_tlm(iPacket);
            }
        } catch (int e) {
            printf("lost connection with the IMU\n");
            if (iPacket) delete iPacket;
            delete imu;
            continue;
        }
    }
}

// this thread creates and sends CamPackets
void camera_thread() {

    CameraPacket* cPacket = nullptr;
    Packet* cmdPacket = nullptr;

    raspicam::RaspiCam camera;
    camera.setWidth(320);
    camera.setHeight(240);
    camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
    camera.setHorizontalFlip(true);
    camera.setVerticalFlip(true);
    camera.setShutterSpeed(1000);

    while (true) {
        // check if there is a command to turn the camera on/off
        if (queue.cmdSize() > 0 && (queue.cmd_front_id() & 0x30) == 0x30) {
            queue.pop_cmd(cmdPacket);
            if (cmdPacket->id == CAM_CMD_ID) {
                CameraPowerCmd* cCmd = static_cast<CameraPowerCmd*>(cmdPacket);
                cCmd->CameraPowerCmd::convert();
                // turn it on
                if (cCmd->state && !camera.isOpened()) {
                    while (!camera.open()) {
                        printf("FAILED to connect to the camera. Trying again in 5 seconds...\n");
                        camera.release();
                        sleep(5);
                        if (queue.cmdSize() > 0 && queue.cmd_front_id() == CAM_CMD_ID) break;
                    }
                    if (queue.cmdSize() > 0 && queue.cmd_front_id() == CAM_CMD_ID) continue;
                    printf("Successfully connected to the camera\n");
                    sleep(2);
                // or turn it off
                } else if (!cCmd->state) {
                    camera.release();
                    printf("Disconnected from the camera\n");
                }
            } else if (cmdPacket->id == CAM_EXP_ID) {
                bool opened = camera.isOpened();
                if (opened) camera.release();
                CameraExpCmd* eCmd = static_cast<CameraExpCmd*>(cmdPacket);
                eCmd->CameraExpCmd::convert();
                camera.setShutterSpeed(eCmd->exposure);
                printf("Camera exposure set to %u microseconds\n", eCmd->exposure);
                if (opened) {
                    camera.open();
                    sleep(2);
                }
            } else if (cmdPacket->id == CAM_ISO_ID) {
                bool opened = camera.isOpened();
                if (opened) camera.release();
                CameraISOCmd* iCmd = static_cast<CameraISOCmd*>(cmdPacket);
                iCmd->CameraISOCmd::convert();
                camera.setISO(iCmd->iso);
                printf("Camera ISO set to %u\n", iCmd->iso);
                if (opened) {
                    camera.open();
                    sleep(2);
                }
            }
            delete cmdPacket;
            cmdPacket = nullptr;
        }

        // get and send the packet
        if (camera.isOpened()) {
            cPacket = new CameraPacket;

            camera.grab();
            cPacket->timestamp = getTimestamp();
            camera.retrieve(cPacket->pBuffer);

            queue.push_tlm(cPacket);
            usleep(20000); // sleep for 20ms
        } else {
            sleep(1);
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
    // launch the instrument threads

    // set high priority for sensor threads
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());

    std::thread(tam_thread).detach();
    std::thread(imu_thread).detach();
    std::thread(camera_thread).detach();

    cmd.str("");
    cmd << "renice -n +1 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());
    std::thread(housekeeping_thread).detach();

    cmd.str("");
    cmd << "renice -n 0 -p " << pid;
    if (pid > 0) system(cmd.str().c_str());

    // the main process gathers and sends GPS packets
    TimePacket* tPacket = nullptr;
    Gps* gps = nullptr;

    while (true) {
        // try opening the GPS every ten seconds until it succeeds
        while (true) {
            try {
                gps = new Gps(1, "/dev/ttyAMA0", 9600);
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
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
