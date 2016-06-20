cmake_minimum_required (VERSION 3.0.2 FATAL_ERROR)
project (COSMOS_Server CXX)

option(USE_FVM400 "Compile with support for FVM400" OFF)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -o3 -Wall")

set (COMMON_LIBS ${EXTRA_LIBS} wiringPi pthread)

set (SENSOR_LIBS raspicam)
#set (SENSOR_LIBS ASICamera)

set (MOTOR_LIBS )

set(COMMON_SOURCES
    gps/gps.cpp
    cosmos/cosmos.cpp
    cosmos/packets.cpp
    cosmos/cosmosQueue.cpp
)

set(SENSOR_SOURCES
    sensorpi.cpp
    imu/imu.cpp
    #camera/camera.cpp
    tam/tam.cpp
    tam/ads1115.cpp
)

set(MOTOR_SOURCES
    motorpi.cpp
    motor/dcmotor.cpp
    motor/pwm.cpp
    motor/decoder.cpp
    motor/pid.cpp
)

if(USE_FVM400)
    set(SENSOR_SOURCES ${SENSOR_SOURCES} fvm400/fvm400.cpp)
    add_definitions(-DUSE_FVM400)
endif()

add_executable (sensor ${COMMON_SOURCES} ${SENSOR_SOURCES})
add_executable (motor ${COMMON_SOURCES} ${MOTOR_SOURCES})

target_link_libraries (sensor ${COMMON_LIBS} ${SENSOR_LIBS})
target_link_libraries (motor ${COMMON_LIBS} ${MOTOR_LIBS})
