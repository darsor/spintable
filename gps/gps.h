#ifndef GPS_H

#include <string>

class Gps {
    private:
        int fd;
        char buffer[10];

    public:
        Gps();
        Gps(std::string device, int baud);
        ~Gps();
        float getTime(); // prints a warning if GPS stream is backlogged
};

#endif
