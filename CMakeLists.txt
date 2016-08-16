cmake_minimum_required (VERSION 3.0.2 FATAL_ERROR)
project (COSMOS_Server CXX)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")

set (COMMON_LIBS ${EXTRA_LIBS} wiringPi pthread)

set (SENSOR_LIBS raspicam)

set (MOTOR_LIBS )

set(COMMON_SOURCES
    src/gps/gps.cpp
    src/cosmos/cosmos.cpp
    src/cosmos/packets.cpp
    src/cosmos/cosmosQueue.cpp
)

set(SENSOR_SOURCES
    src/sensorpi.cpp
    src/imu/imu.cpp
    src/tam/tam.cpp
    src/tam/ads1115.cpp
)

set(MOTOR_SOURCES
    src/motorpi.cpp
    src/motor/dcmotor.cpp
    src/motor/pwm.cpp
    src/motor/decoder.cpp
    src/motor/pid.cpp
)

add_executable (sensor ${COMMON_SOURCES} ${SENSOR_SOURCES})
add_executable (motor ${COMMON_SOURCES} ${MOTOR_SOURCES})

target_link_libraries (sensor ${COMMON_LIBS} ${SENSOR_LIBS})
target_link_libraries (motor ${COMMON_LIBS} ${MOTOR_LIBS})
