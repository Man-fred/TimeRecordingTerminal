#include "log.h"
#include "defines.h"
#include <LittleFS.h>
#include <EEPROM.h>


// if (LittleFS.remove("/Daten.txt") ) Serial.println("Datei Daten.txt gelöscht");

void DateiZuGross()
{
  File DataFile = LittleFS.open("/log.txt", "r");
  if (DataFile.size() > 30000) // Wenn Dateigrösse 30Kb überschreitet
  {
    DataFile.seek(-10000, SeekEnd); // Letzte 10Kb in neue Datei übernehmen
    DataFile.readStringUntil('\r'); // Zeiger auf Zeilenanfang plazieren
    
    File DataFile1 = LittleFS.open("/logNeu.txt", "w");
    Serial.println("logNeu.txt Erzeugt");
    while (DataFile.available()) DataFile1.print(DataFile.readStringUntil('\n')); // Vom Zeiger bis Ende "/log.txt" in "/logNeu.txt" kopieren
    DataFile.close();
    DataFile1.close();
    if (LittleFS.remove("/log.txt") ) Serial.println("Datei log.txt gelöscht");
    if (LittleFS.rename("/logNeu.txt", "/log.txt") ) Serial.println("Datei logNeu.txt zu log.txt umbenant");
    return;
  }
  DataFile.close();
}

boolean LogSchreiben(String Daten)
{
  if (!LittleFS.exists("/log.txt"))
  {
    File DataFile = LittleFS.open("/log.txt", "w");
    DataFile.println(Daten);
    DataFile.close();
    return true;
  }
  DateiZuGross(); // Überprüfen ob die log.txt nicht zu gross wird
  File DataFile = LittleFS.open("/log.txt", "a");
  DataFile.println(Daten); // + "\r\n"
  DataFile.close();
  return true;
}

boolean LogSchreibenNow(String Daten) {
  //String Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   " + Daten;
  return LogSchreiben(PrintDate(now()) + "   " + PrintTime (now()) + "   " + Daten);
}




