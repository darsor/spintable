#include "tam.h"
#include <cstdio>
#include <unistd.h>

int main() {

    Tam tammy;
    float voltage;

    for (;;) {
        voltage = tammy.getData(0) * (4.096 / 32768);
        printf("reading: %-6f\n", voltage);
        usleep(10000);
    }

    return 0;
}
