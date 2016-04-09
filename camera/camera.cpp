#include "camera.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

Camera::Camera() {
    /***************************************************
    here is the suggested procedure to operate the camera.

        --> getNumOfConnectedCameras
        --> openCamera
        --> setImageFormat
        --> initCamera
        --> setValue (for wanted values)
        --> startCapture

        //this is recommended to do in another thread
        while(1)
        {
            getImageData
                ...
        }
    ***************************************************/
    int numDevices = getNumberOfConnectedCameras();
    //int numControls = 0;

    if (numDevices <= 0) {
        printf("no camera connected, press any key to exit\n");
        getchar();
        exit(1);
    } else printf("attached cameras:\n");

    if (!openCamera(camNum)) {
        printf("ERROR opening camera, are you root? Press any key to exit\n");
        getchar();
        exit(1);
    } else printf("opened camera successfully\n");

    int bin = 2;
    int imageType = 0;
    int resX = 320;
    int resY = 320;

    initCamera();
    if(setImageFormat(resX, resY, bin, (IMG_TYPE)imageType)) {
        printf("\ncamera formatted correctly\n");
    } else {
        printf("camera not formatted correctly, press any key to comtinue\n");
        getchar();
        exit(1);
    }

    // These controls were found in the SDK demo
    setValue(CONTROL_EXPOSURE, 1000, false); //ms//auto
    setValue(CONTROL_GAIN, 0, false); 
    setValue(CONTROL_GAMMA, 1, false); 
    setValue(CONTROL_BANDWIDTHOVERLOAD, getMin(CONTROL_BANDWIDTHOVERLOAD), false); //low transfer speed

    setValue(CONTROL_WB_B, 90, false);
    setValue(CONTROL_WB_R, 48, false);
//  setAutoPara(getMax(CONTROL_GAIN)/2,10,150); //max auto gain and exposure and target brightness
//	EnableDarkSubtract("dark.bmp"); //dark subtract will be disabled when exposure set auto and exposure below 500ms
    startCapture(); //start preview
}

Camera::~Camera() {
    stopCapture();
    closeCamera();
}

int Camera::getFrame(unsigned char (&pBuffer)[102400]) {
    if (getImageData(pBuffer, sizeof(pBuffer), -1)) {
        return 0;
    } else return -1;
}
