#ifndef NTP_H
#define NTP_H

#include "Arduino.h"

#include <TimeLib.h>
#include <WiFiUdp.h>
#define TIMEZONE 1
#define SUMMERTIME 1

boolean summertime(int year, byte month, byte day, byte hour, byte tzHours);


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address);


/**
   NTP senden und empfangen
*/
unsigned long GetNTP(void);

String PrintTime (unsigned long epoch);


String PrintDate (unsigned long epoch);


boolean sommerzeitTest(); 

boolean feiertag(time_t test);


int berechne_Ostern();

boolean buss_und_bettag();

#endif
