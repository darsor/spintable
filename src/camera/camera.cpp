#include "camera.h"
#include <cstdlib>
#include <cstdio>


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
    }

    if (!openCamera(camNum)) {
        printf("ERROR opening camera, are you root? Press any key to exit\n");
        getchar();
        exit(1);
    } else printf("opened camera successfully\n");

    int bin = 1;
    int imageType = 0;
    int resX = 320;
    int resY = 240;

    initCamera();
    if(setImageFormat(resX, resY, bin, (IMG_TYPE)imageType)) {
    } else {
        printf("camera not formatted correctly, press any key to continue\n");
        getchar();
        exit(1);
    }

    // These controls were found in the SDK demo
#ifndef AUTO_EXP
    setValue(CONTROL_EXPOSURE, 400, false); //us//auto
    setValue(CONTROL_GAIN, 0, false); 
    setValue(CONTROL_GAMMA, 1, false); 
#else
    setAutoPara(getMax(CONTROL_GAIN)/2,10,150); //max auto gain and exposure and target brightness
#endif

    setValue(CONTROL_BANDWIDTHOVERLOAD, getMin(CONTROL_BANDWIDTHOVERLOAD), false); //low transfer speed
    setValue(CONTROL_WB_B, 90, false);
    setValue(CONTROL_WB_R, 48, false);
//	EnableDarkSubtract("dark.bmp"); //dark subtract will be disabled when exposure set auto and exposure below 500ms
    started = false;
}

Camera::~Camera() {
    stopCapture();
    closeCamera();
}

void Camera::start() {
    startCapture();
    started = true;
}

void Camera::stop() {
    stopCapture();
    started = false;
}

void Camera::setExposure(unsigned int us) {
    setValue(CONTROL_EXPOSURE, us, false); //us//auto
}

int Camera::getFrame(unsigned char* pBuffer) {
    if (getImageData(pBuffer, 76800, -1)) {
        return 0;
    } else return -1;
}
