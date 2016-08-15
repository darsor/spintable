#include "camera2.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

Camera::Camera() {
    /***************************************************
    here is the suggested procedure to operate the camera.

        --> ASIGetNumOfConnectedCameras
        ----> ASIGetCameraProperty for each camera

        --> ASIOpenCamera
        --> ASIGetNumOfControls
        ----> ASIGetControlCaps for each contronl and set or get value from them

        --> ASISetROIFormat

        --> ASIStartVideoCapture

        //this is recommended to do in another thread
        while(1)
        {
            ASIGetVideoData
                ...
        }
    ***************************************************/
    int numDevices = ASIGetNumOfConnectedCameras();
    int numControls = 0;

    if (numDevices <= 0) {
        printf("no camera connected, press any key to exit\n");
        getchar();
        exit(1);
    } else printf("attached cameras:\n");

    ASI_CAMERA_INFO ASICameraInfo;

    for (int i=0; i<numDevices; i++) {
        ASIGetCameraProperty(&ASICameraInfo, i);
        printf("%d %s\n", i, ASICameraInfo.Name);
    }

    if (ASIOpenCamera(camNum) != ASI_SUCCESS) {
        printf("ERROR opening camera, are you root? Press any key to exit\n");
        getchar();
        exit(1);
    } else printf("opened camera successfully\n");

    printf("max resolution: %d x %d\n", ASICameraInfo.MaxWidth, ASICameraInfo.MaxHeight);
    if (ASICameraInfo.SupportedVideoFormat[0] == ASI_IMG_RAW8) {
        printf("video format: RAW8\n");
    }

    ASI_CONTROL_CAPS controlCaps;
    ASIGetNumOfControls(camNum, &numControls);
    printf("\nAvailable Settings:\n");
    for (int i=0; i<numControls; i++) {
        ASIGetControlCaps(camNum, i, &controlCaps);
        printf("%s\n", controlCaps.Name);
    }

    int bin = 2;
    int imageType = 0;
    int resX = 320;
    int resY = 320;

    if (ASI_SUCCESS == ASISetROIFormat(camNum, resX, resY, bin, (ASI_IMG_TYPE) imageType)) {
        printf("\ncamera formatted correctly\n");
    } else {
        printf("camera not formatted correctly, press any key to comtinue\n");
        getchar();
        exit(1);
    }

    if (ASI_SUCCESS == ASIStartVideoCapture(camNum)) {
        printf("started video capture\n");
    } else {
        printf("unable to start video capture\n");
        getchar();
        exit(1);
    }
}

Camera::~Camera() {
    ASICloseCamera(camNum);
}

int Camera::getFrame(unsigned char (&pBuffer)[102400]) {
    if (ASI_SUCCESS == ASIGetVideoData(camNum, pBuffer, sizeof(pBuffer), 1000))
        return 0;
    else return -1;
}
