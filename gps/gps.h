#ifndef GPS_H

#include <stdint.h>

struct gpsTime {
    uint16_t hhmmss;
    uint16_t sss;
};

class Gps {
    private:
        int fd;
        char buffer[10];

    public:
        Gps();
        Gps(char* device, int baud);
        ~Gps();
        gpsTime getTime(); // prints a warning if GPS stream is backlogged
};

#endif
