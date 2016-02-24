#ifndef CAMERA_H
#define CAMERA_H

#include "ASICamera2.h"

class Camera {
    public:
        Camera();
        ~Camera();
        int getFrame(unsigned char (&pBuffer)[307200]);
    private:
        int camNum = 0;
};

#endif
