#ifndef CAMERA_H
#define CAMERA_H

#include "ASICamera.h"

class Camera {
    public:
        Camera();
        ~Camera();
        int getFrame(unsigned char* pBuffer);
        void start();
        void stop();
        bool isStarted() { return started; }
        void setExposure(unsigned int us);
    private:
        int camNum = 0;
        bool started;
};

#endif
