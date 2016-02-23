#include <termios.h> // terminal io (serial port) interface
#include <fcntl.h>   // File control definitions
#include <errno.h>   // Error number definitions
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <string>
#include "imu.h"

// Clears the com port's read and write buffers
int Purge(ComPortHandle comPortHandle){
    if (tcflush(comPortHandle,TCIOFLUSH)==-1){
        printf("flush failed\n");
        return 0;
    }
    return 1;
}

// Opens a com port with the correct settings for communicating with a MicroStrain 3DM-GX3-25 sensor
ComPortHandle OpenComPort(const char* comPortPath){
    ComPortHandle comPort = open(comPortPath, O_RDWR | O_NOCTTY);
    if (comPort== -1){ //Opening of port failed
        printf("Unable to open com Port %s\n Errno = %i\n", comPortPath, errno);
        return -1;
    }
    //Get the current options for the port...
    struct termios options;
    tcgetattr(comPort, &options);
    //set the baud rate to 115200
    int baudRate = B115200;
    cfsetospeed(&options, baudRate);
    cfsetispeed(&options, baudRate);
    //set the number of data bits.
    options.c_cflag &= ~CSIZE;    // Mask the character size bits
    options.c_cflag |= CS8;
    //set the number of stop bits to 1
    options.c_cflag &= ~CSTOPB;
    //Set parity to None
    options.c_cflag &=~PARENB;
    //set for non-canonical (raw processing, no echo, etc.)
    options.c_iflag = IGNPAR; // ignore parity check close_port(int
    options.c_oflag = 0; // raw output
    options.c_lflag = 0; // raw input
    //Time-Outs -- won't work with NDELAY option in the call to open
    options.c_cc[VMIN]  = 0;      // block reading until RX x characers. If x = 0, it is non-blocking.
    options.c_cc[VTIME] = 10;     // Inter-Character Timer -- i.e. timeout= x*.1 s
    //Set local mode and enable the receiver
    options.c_cflag |= (CLOCAL | CREAD);
    //Purge serial port buffers
    Purge(comPort);
    //Set the new options for the port...
    int status=tcsetattr(comPort, TCSANOW, &options);
    if (status != 0){ //For error message
        printf("Configuring comport failed\n");
        return status;
    }
    //Purge serial port buffers
    Purge(comPort);
    return comPort;
}

// Closes a port that was previously opened with OpenComPort
void CloseComPort(ComPortHandle comPort){
    close(comPort);
}

// readComPort
// read the specivied number of bytes from the com port
int readComPort(ComPortHandle comPort, Byte* bytes, int bytesToRead){
    int bytesRead = read(comPort, bytes, bytesToRead);
    return bytesRead;     
}

// writeComPort
// send bytes to the com port
int writeComPort(ComPortHandle comPort, unsigned char* bytesToWrite, int size){
    return write(comPort, bytesToWrite, size);
}

//scandev
//finds attached microstrain devices
char* scandev(){
    FILE *instream;
    char devnames[255][255];//allows for up to 256 devices with path links up to 255 characters long each
    int devct=0; //counter for number of devices
    int userchoice=0;
    char* device;
    std::string str = "find /dev/serial -print | grep -i ftdi";
    const char *command = str.c_str(); // search /dev/serial for microstrain devices
    instream=popen(command, "r"); // execute piped command in read mode
    if(!instream){//SOMETHING WRONG WITH THE SYSTEM COMMAND PIPE...EXITING
        printf("ERROR BROKEN PIPELINE %s\n", command);
        return device;
    }
    for(int i=0;i<255&&(fgets(devnames[i],sizeof(devnames[i]), instream));i++){//load char array of device addresses
        ++devct;
    }
    for(int i=0;i<devct;i++){
        for(int j=0;j<sizeof(devnames[i]);j++){
            if(devnames[i][j]=='\n'){
                devnames[i][j]='\0';//replaces newline inserted by pipe reader with char array terminator character 
                break;//breaks loop after replacement
            }
        }
    }

    if(devct>0){
        device=devnames[userchoice];
        return device;
    }
    else{
        printf("No MicroStrain devices found.\n");
        return device;
    }
}


Imu::Imu(){
    char* dev;
    char a;
    a='\0';
    dev=&a;
        dev=scandev();
        if(strcmp(dev,"")!=0){
            printf("Attempting to open port...%s: ",dev);
            comPort = OpenComPort(dev);
        }
        else{
            printf("Failed to find attached device.\n");
            return;
        }
    if(comPort > 0)  
    printf("connected. \n\n");
}

void Imu::getdata(Byte (&data)[43]) {
    int size;
    writeComPort(comPort, &dataCommand, 1); // write command to port
    Purge(comPort); // flush port

    size = readComPort(comPort, &data[0], 43);
    while (size!=43) {
        size = size + readComPort(comPort, &data[size], 43);
    }
    if(size<=0){
        printf("No data read from IMU\n");
    }
}

void Imu::getQuaternion(Byte (&data)[23]) {
    int size;
    writeComPort(comPort, &quatCommand, 1); // write command to port
    Purge(comPort); // flush port

    size = readComPort(comPort, &data[0], 23);
    while (size!=23) {
        size = size + readComPort(comPort, &data[size], 23);
    }
    if(size<=0){
        printf("No data read from IMU\n");
    }
}

Imu::~Imu(){
    CloseComPort(comPort);
}
