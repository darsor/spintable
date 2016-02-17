#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

int main() {

    int fd;
    char buffer[10];
    double time;
    
    if ((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0) {
        perror("The following error occured");
        return 1;
    }
    else printf("device opened\n");

    if (wiringPiSetup() < 0) {
        perror("The following error occured");
        return 1;
    }

   delay(5);

    for (;;) {
        while ( ((char) serialGetchar(fd)) != '$');
        while ( ((char) serialGetchar(fd)) != ',');
        for (int i=0; i<10; i++) {
            buffer[i] = (char) serialGetchar(fd);
            time = atof(buffer);
        }
        printf("%f\n",time);
    }

    serialClose(fd);
    printf("device closed\n");
    return 0;
}
