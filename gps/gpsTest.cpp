#include "gps.h"
#include <unistd.h>

int main() {
    Gps gps;

    while(true) {
        printf("time: %d\n", gps.getTime());
        usleep(2000000);
    }
    return 0;
}
