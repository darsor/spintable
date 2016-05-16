#include "cosmos/cosmos_queue.h"
#include "gps/gps.h"
#include "camera/camera.h"
#include "tam/tam.h"
#include "imu/imu.h"
#include <wiringPi.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <cerrno>
#include <mutex>
#include <condition_variable>

// gather data from various sensors
void imu(SensorPacket* p);
void tam(SensorPacket* p);
void camera(CameraPacket* p);
void gps(TimePacket* p);
void systemTimestamp(uint32_t &stime, uint32_t &ustime);

// set up condition variables to wake the TAM thread
std::mutex m;
std::condition_variable cv;
bool tamStart = false;

// initialize packets
TimePacket* tPacket = NULL;
SensorPacket* sPacket = NULL;

// make the CosmosQueue global (so that all threads can access it)
CosmosQueue<Packet*> queue(128);

PI_THREAD (cameraControl) {
    static CameraPacket* cPacket = NULL;
    camera(cPacket);
    while (true) {
        cPacket = new CameraPacket();
        systemTimestamp(cPacket->sysTimeSeconds, cPacket->sysTimeuSeconds);
        camera(cPacket);
        queue.push(cPacket);
    }
}

PI_THREAD (tamControl) {
    while (true) {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return tamStart;});
        tam(sPacket);
        tamStart = false;
        lk.unlock();
        cv.notify_one();
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

    // initialize devices
    imu(sPacket);
    tam(sPacket);
    gps(tPacket);

    // start threads
    if (piThreadCreate(cameraControl) != 0) {
        perror("Camera control thread didn't start");
    }
    if (piThreadCreate(tamControl) != 0) {
        perror("TAM control thread didn't start");
    }
    if (piThreadCreate(cosmosQueue) != 0) {
        perror("COSMOS queue thread didn't start");
    }

    // set high priority for this thread
    pid_t pid = getpid();
    std::stringstream cmd;
    cmd << "renice -n -2 -p " << pid;
    if (pid > 0)
        system(cmd.str().c_str());

    long timer = 0;
    long difference = 0;
    struct timeval start, next;
    while (true) {

        // get timestamps and send time packet
        tPacket = new TimePacket();
        gps(tPacket);
        systemTimestamp(tPacket->sysTimeSeconds, tPacket->sysTimeuSeconds);
        gettimeofday(&start, NULL);
        queue.push(tPacket);

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
            } while (difference < timer);
            if (difference < timer) usleep(100);
            if (difference > 982000) break;
            //printf("started cycle at %li/%li\n", difference, timer);

            sPacket = new SensorPacket();
            { // tell the TAM to collect data
                std::lock_guard<std::mutex> lk(m);
                tamStart = true;
                systemTimestamp(sPacket->sysTimeSeconds, sPacket->sysTimeuSeconds);
            } cv.notify_one();
            imu(sPacket);
            { // wait for TAM to finish
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk, []{return !tamStart;});
            }
            queue.push(sPacket);

            timer += 20000;
        }
    }
    return 0;
}

void imu(SensorPacket* p) {
    static Imu imuSensor;
    if (p == NULL) return;
    imuSensor.getdata(p->imuData);
    imuSensor.getQuaternion(p->imuQuat);
}

void tam(SensorPacket* p) {
    static Tam tamSensor;
    if (p == NULL) return;
    p->tamA = tamSensor.getData(0);
    p->tamB = tamSensor.getData(1);
    p->tamC = tamSensor.getData(2);
}

void camera(CameraPacket* p) {
    static Camera cam;
    if (p == NULL) return;
    cam.getFrame(p->pBuffer);
}

void gps(TimePacket* p) {
    static Gps gps("/dev/ttyAMA0", 9600);
    if (p == NULL) return;
    p->gpsTime = gps.getTime();
}

void systemTimestamp(uint32_t &stime, uint32_t &ustime) {
    static struct timeval timeVal;
    gettimeofday(&timeVal, NULL);
    stime =  timeVal.tv_sec;
    ustime = timeVal.tv_usec;
}
