#include <wiringPiSPI.h>
#include <cstdio>

int setup_encoder(unsigned char (&)[10]);
int read_encoder(unsigned char (&)[10]);
int clear_encoder(unsigned char (&)[10]);

int main(){
    int feedback=wiringPiSPISetup(0,9600);
    unsigned char data[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    
    setup_encoder(data);
    read_encoder(data);
    for(int i=0; i<10; i++)
        printf("%c",data[i]);
    printf("\n");
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

int read_encoder(unsigned char (&data)[10]){
    data[0]=0x60; 
    data[1]=0x00;
    data[2]=0x00;
    data[3]=0x00;
    data[4]=0x00;

    for(int i=0; i<10; i++)
        wiringPiSPIDataRW(0, &data[i], 5);
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
