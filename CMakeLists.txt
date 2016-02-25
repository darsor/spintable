cmake_minimum_required (VERSION 3.0.2 FATAL_ERROR)
project (COSMOS_Server CXX)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -o3")

set (EXTRA_LIBS ${EXTRA_LIBS}
    wiringPi
    ASICamera2
)

add_executable (COSMOS_Server
    server.cpp
    gps/gps.cpp
    imu/imu.cpp
    camera/camera.cpp
)

target_link_libraries (COSMOS_Server ${EXTRA_LIBS})
