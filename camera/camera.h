#ifndef CAMERA_H
#define CAMERA_H

#include "ASICamera.h"

class Camera {
    public:
        Camera();
        ~Camera();
        int getFrame(unsigned char (&pBuffer)[102400]);
    private:
        int camNum = 0;
};

#endif
