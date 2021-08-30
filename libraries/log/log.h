#ifndef LOG_INO_H
#define LOG_INO_H

#include "ntp.h"


// if (SPIFFS.remove("/Daten.txt") ) Serial.println("Datei Daten.txt gelöscht");

void DateiZuGross();
boolean LogSchreiben(String Daten);
boolean LogSchreibenNow(String Daten);

#endif


