# TimeRecordingTerminal
time recording with esp8266 to a socket connection.

First commit with OTA-Updates. The version is stable with following functions:
 - esp8266
 - LCD - display 4 x 20
 - 4 x 4 keypad
 - NTP with RTC-fallback
 - Mifare-rfid scanning online and offline (offline is temporary saved in esp8266-LittleFS)
 - OTA with webserver: the files from www must be located in /esp8266/ and php must be activated in the webserver.
 - An answer is expected from the socket connection an is printed on the display.
 
 Description of hardware and function is coming later. 
 
 Files for 3D-printing follows.
