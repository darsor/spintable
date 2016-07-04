//Header file for the GPS class.
#ifndef GPS_H
#define GPS_H

#include <string>
#include <unistd.h>

class Gps {
    private:
        int ppsPin;
        int fd;
        char buffer[10];

    public:
        Gps(int ppsPin=1, std::string device="/dev/ttyAMA0", int baud=9600);
        ~Gps();
        float getTime(); // blocks until the next timestamp sent from GPS
        void timestampPPS(uint32_t &stime, uint32_t &ustime);
        bool dataAvail();
};

#endif
