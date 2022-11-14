#ifndef DEFINES_H
#define DEFINES_H

#include "Arduino.h"

// ********** Globale Einstellungen zum Layout und zur Version **************
#define IICTEST
#define IIC_RTC
#define IIC_KEYPAD
#define IIC_DISPLAY
#define USE_LED_BUILTIN

#define SPITEST
#define SPI_RFID

#define USE_HTTPS

// ********** Ende der Einstellungen ****************************************

//??//#include <TimeLib.h>             //<Time.h> http://www.arduino.cc/playground/Code/Time
#define DEBUG_OUTPUT Serial
#define DBG_OUTPUT_PORT Serial

// I2C
# define IO_I2C_ADDRESS 0x20     
# define RTC_I2C_ADDRESS 0x68
// oled 0,91 # define DISPLAY_I2C_ADDRESS 0xFF
# define DISPLAY_I2C_ADDRESS 0x27

#define LOGINLAENGE 32

#endif
