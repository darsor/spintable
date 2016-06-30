#include "fvm400.h"
#include <wiringSerial.h> // for serial read/write commands
#include <unistd.h> // for close()
#include <cstdlib> // for atoi()
#include <cstring> // for strchr()
#include <cstdio> // for printf() and perror()

FVM400::FVM400(std::string device) {
    fd = serialOpen(device.c_str(), 9600);
    if (fd < 0) {
        perror("ERROR opening FVM400");
        exit(1);
    }
    setDefault();
}

FVM400::~FVM400() {
    close(fd);
}

FVM400_data FVM400::getData() {
    struct FVM400_data data;
    char response;
    char* index;
    char buffer[128];
    serialFlush(fd);
    serialPutchar(fd, '?');
    response = serialGetchar(fd);
    if (response != 'A') {
        printf("getData command not registered\n");
        return data;
    }
    serialGetchar(fd);
    for (int i=0; response != 0x04; i++) {
        response = serialGetchar(fd);
        buffer[i] = response;
        if (response == 0x04) buffer[i+1] = '\0';
    }
    index = buffer;
    data.comp1 = atoi(index);
    index = strchr(index, ',') + 1;
    data.comp2 = atoi(index);
    index = strchr(index, ',') + 1;
    data.comp3 = atoi(index);
    return data;
}

int FVM400::getMode() {
    char response;
    serialFlush(fd);
    serialPutchar(fd, 'G');
    serialPutchar(fd, 'M');
    response = serialGetchar(fd);
    if (response != 'A') {
        printf("getMode comand not registered\n");
        return -1;
    }
    response = serialGetchar(fd);
    if (response == '0') return 0;
    else if (response == '1') return 1;
    else return -1;
}

int FVM400::getCoordMode() {
    char response;
    serialFlush(fd);
    serialPutchar(fd, 'G');
    serialPutchar(fd, 'X');
    response = serialGetchar(fd);
    if (response != 'A') {
        printf("getMode comand not registered\n");
        return -1;
    }
    response = serialGetchar(fd);
    if (response == '0') return 0;
    else if (response == '1') return 1;
    else return -1;
}

void FVM400::setMode(int mode) {
    serialFlush(fd);
    if (mode == FVM400_ABSOLUTE_MODE) {
        char cmd[4] = {'S', 'M', '0', '\0'};
        serialPuts(fd, cmd);
    } else if (mode == FVM400_RELATIVE_MODE) {
        char cmd[4] = {'S', 'M', '1', '\0'};
        serialPuts(fd, cmd);
    } else {
        printf("setMode: invalid mode\n");
        return;
    }
    if (serialGetchar(fd) != 'A') printf("setMode failed\n");
    else printf("setMode success\n");
}

void FVM400::setCoordMode(int coord_mode) {
    serialFlush(fd);
    if (coord_mode == FVM400_RECT_COORD) {
        char cmd[4] = {'S', 'X', '0', '\0'};
        serialPuts(fd, cmd);
    } else if (coord_mode == FVM400_POLAR_COORD) {
        char cmd[4] = {'S', 'X', '1', '\0'};
        serialPuts(fd, cmd);
    } else {
        printf("setCoordMode: invalid mode\n");
        return;
    }
    if (serialGetchar(fd) != 'A') printf("setCoordMode failed\n");
    else printf("setCoordMode success\n");
}

void FVM400::setDefault() {
    serialFlush(fd);
    serialPutchar(fd, '*');
    if (serialGetchar(fd) != 'A') {
        printf("setDefault failed\n");
        return;
    } else printf("setDefault success\n");
}
