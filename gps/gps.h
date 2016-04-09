//Header file for the GPS class.
#ifndef GPS_H
#define GPS_H

#include <string>
#include <unistd.h>

class Gps {
    private:
        int fd;
        char buffer[10];

    public:
        Gps();
        Gps(std::string device, int baud);
        ~Gps();
        uint32_t getTime(); // blocks until the next timestamp sent from GPS
};

#endif
