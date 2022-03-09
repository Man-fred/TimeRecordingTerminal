# TimeRecordingTerminal
time recording with esp8266 to a socket connection.

First commit with OTA-Updates. The version is stable with following functions:
 - esp8266
 - LCD - display 4 x 20
 - 4 x 4 keypad
 - NTP with RTC-fallback
 - Mifare-rfid scanning online and offline (offline is temporary saved in esp8266-LittleFS)
 - OTA with webserver: the files from www must be located in /esp8266/ and php must be activated in the webserver. But OTA is unnecessary for the function of TimeRecordingTerminal.
 - An answer is expected from the socket connection an is printed on the display.
 
Compiled with Arduino 1.8.12 and esp8266 version 3.0.2

Used libraries are included and should be connected in Windows to the Arduino software with a junction: mklink /j <arduino-folder>\libraries <TimeRecordingTerminal-folder>\libraries. You can test if the right library is integrated with "dir /a"
 
 Description of hardware and function is coming later. 
 
 Files for 3D-printing follows.
