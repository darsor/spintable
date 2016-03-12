#ifndef CAMERA2_H
#define CAMERA2_H

#include "ASICamera2.h"

class Camera {
    public:
        Camera();
        ~Camera();
        int getFrame(unsigned char (&pBuffer)[102400]);
    private:
        int camNum = 0;
};

#endif
