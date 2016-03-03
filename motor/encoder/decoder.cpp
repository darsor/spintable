#include <wiringPiSPI.h>
#include <cstdio>
#include <cstdlib>

int setup_encoder(unsigned char (&)[10]);
int read_encoder(unsigned char data[]);
int clear_encoder(unsigned char (&)[10]);

int main(){
    system("gpio load spi");
    int feedback=wiringPiSPISetup(0,9600);
    unsigned char data[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    setup_encoder(data);
    for(int i=0; i<10; i++)
    read_encoder(data);
    clear_encoder(data);

    for(int i=0; i<10; i++)
        printf("%c",data[i]);
    printf("\n");

    return 0;
}

int setup_encoder(unsigned char (&data)[10]){
    data[0]=0x88;
    data[1]=0x03;

    for(int i=0; i<10; i++)
        wiringPiSPIDataRW(0, &data[i], 2);
}

int read_encoder(unsigned char data[]){
    data[0]=0x60; 
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    data[4]=0x00;

    //for(int i=0; i<10; i++)
        wiringPiSPIDataRW(0, data, 1);
    for(int i=0; i<10; i++)
        printf("%x",data[i]);
    printf("\n");
}

int clear_encoder(unsigned char (&data)[10]){
    data[0]=0x98;
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    data[4]=0x00;

    for(int i=0; i<10; i++)
        wiringPiSPIDataRW(0, &data[i], 5);
}
