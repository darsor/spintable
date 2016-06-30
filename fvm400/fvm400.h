#ifndef FVM400_H
#define FVM400_H

#define FVM400_RELATIVE_MODE    0
#define FVM400_ABSOLUTE_MODE    1
#define FVM400_RECT_COORD       0
#define FVM400_POLAR_COORD      1

#include <string>
#include <cstdint>

struct FVM400_data {
    int32_t comp1;
    int32_t comp2;
    int32_t comp3;
};

class FVM400 {
    public:
        FVM400(std::string device);
        ~FVM400();

        FVM400_data getData();
        int getMode();
        int getCoordMode();

        void setMode(int mode);
        void setCoordMode(int coord_mode);

        void setDefault(); // rect coord, absolute mode

    private:
        int fd;
};

#endif
