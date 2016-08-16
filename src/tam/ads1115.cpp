/*
 * ADS1115 driver for wiringPi:
 *    https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 * March 2015 John Whittington http://www.jbrengineering.co.uk @j_whittington
 *
*==============================================================*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <unistd.h>

#include "ads1115.h"

/* ADS1115 analogRead function
*===================================*/

static int myAnalogRead(struct wiringPiNodeStruct *node, int pin) {
  int chan = pin - node->pinBase;
  int16_t value;

  // Start with default values
  int config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
               ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
               ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
               ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
               ADS1115_REG_CONFIG_DR_475SPS    | // 475 samples per second
               ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)
             //ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous mode (doesn't work with more than one channel)
  // Set PGA/voltage range
  config |= ADS1015_REG_CONFIG_PGA_6_144V;

  // Set single-ended input chan
  switch (chan)
  {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;
  
  // Sent the config data in the right order
  config = ((config >> 8) & 0x00FF) | ((config << 8) & 0xFF00);
  wiringPiI2CWriteReg16(node->fd, ADS1015_REG_POINTER_CONFIG, config);
 
  // Wait for conversion to complete
  delay(2); // (1/SPS rounded up)
  //usleep(1500);

  //wiringPiI2CWrite(node->fd, ADS1015_REG_POINTER_CONVERT);
  value = wiringPiI2CReadReg16 (node->fd, ADS1015_REG_POINTER_CONVERT);
  value = ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
  //if (chan == 0) printf("TAM: %-6i (%02x%02x)\n", value, (value & 0xFF00) >> 8, value & 0x00FF);
  return value;
}

/* ADS1115 ADC setup:
 *    create ADS1115 device.
 *    id is the address of the chip (0x48 default)
*===============================================*/

int ads1115Setup(const int pinBase, int id) {
  struct wiringPiNodeStruct *node;

  //system("gpio load i2c 1000");

  node = wiringPiNewNode(pinBase,4);

  node->fd = wiringPiI2CSetup(id);
  node->analogRead = myAnalogRead;

  if (node->fd < 0) {
    throw 1;
    return -1;
  } else {
    return 0;
  }
}
