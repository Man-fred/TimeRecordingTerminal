#ifndef COMMON_H
#define COMMON_H

#include "Arduino.h"
#include "defines.h"

String mVersionNr = "V00-00-00.";
#ifdef IICTEST
# ifdef USE_LED_BUILTIN
    String mVersionVariante = "iic.";
# else
    String mVersionVariante = "i2c.";
# endif
#else
  String mVersionVariante = "min.";
#endif //ifdef IICTEST

#ifdef ARDUINO_ESP8266_NODEMCU
  const byte board = 1;
  String mVersionBoard = "nodemcu";
#elif ARDUINO_ESP8266_WEMOS_D1MINI
  const byte board = 2;
  String mVersionBoard = "d1_mini";
#else
  const byte board = 3;
  String mVersionBoard = "unknown";
#endif
//EEPROM-Version
  char versionNeu[2] = "1";

// enables OTA updates
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server;

const char * headerKeys[] = {"User-Agent","Set-Cookie","Cookie","Date","Content-Type","Connection"} ;
size_t headerKeysCount = 6;

char device_name[LOGINLAENGE] = "Zeiterfassung";
char name_r[4][LOGINLAENGE]; 
char UpdateServer[LOGINLAENGE] = "192.168.178.60\0";
char timeserver[LOGINLAENGE] = "192.168.178.1\0";    //"time.nist.gov\0";
int  UserCookie[COOKIE_MAX];// = [0,0,0,0,0,0,0,0,0,0];
int  UserStatus[COOKIE_MAX];// = [0,0,0,0,0,0,0,0,0,0];
int  UserNext=0;
int  UserCurrent = -1;

const char* serverIndex = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>";

// Timer
unsigned long NTPTime = 0, RTCTime = 0, RTCSync = 0, ZeitTemp = 0, ZeitTempMin = 0, ZeitTempStd = 0, ZeitTempTag = 0;
// Status
byte NTPok = 0, WLANok = 0, IOok = 0, RTCok = 0, DISPLAYok = 0;
boolean AP = 0; // Accesspoint Modus aus

#endif
