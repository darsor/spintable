#include <libgpsmm.h>
#include <iostream>

int main(void)
{
    time_t seconds;
    timestamp_t ts;
    std::string str = "2948";
    const char* port = str.c_str();
    struct gps_data_t* newdata;

    gpsmm gps_rec("localhost", port);

    if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
        std::cerr << "No GPSD running.\n";
        return 1;
    }

    for (;;) {

        if (!gps_rec.waiting(50000000))
          continue;

        if ((newdata = gps_rec.read()) == NULL) {
            std::cerr << "Read error.\n";
            return 1;
        } else {
            //PROCESS(newdata);
            ts = newdata->online;
            seconds = (time_t) ts;
            printf("Time: %s\n",ctime(&seconds));
        }
    }
    return 0;
}
