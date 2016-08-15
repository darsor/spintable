#include "camera.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>

int main() {
    Camera cam;
    unsigned char pBuffer[102400];

    cam.getFrame(pBuffer);

    std::ofstream ofile("image", std::ios::binary);
    printf("file is%s open\n", ofile.is_open() ? " " : " not");
    ofile.write((char*) pBuffer, sizeof(pBuffer));
    ofile.close();
    std::system("convert -depth 8 -size 320x320+0 gray:image out.png");
    std::system("sudo rm image");
    return 0;
}
