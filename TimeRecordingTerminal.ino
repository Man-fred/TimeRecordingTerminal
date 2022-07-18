// Basic MFRC522 RFID Reader Code by cooper @ my.makesmart.net
// Released under Creative Commons - CC by cooper@my.makesmart.net
#include "defines.h"
#define WEBSERVER
//Update-Version
const String mVersionNr = "V";
char hardware[5]= "D300";
char versionNr[30] = "V01-00-01.tr2.d1_mini";
//EEPROM-Version
char versionNeu[2] = "4";

//Sketch
//String mVersionVariante = "tr2.";

char keypass[21];
char www_password[21] = "esp8266";

char ssid[32] = "";
char passwort[64] = "";
char serverHost[LOGINLAENGE] = ""; //IP des Servers 

#include <stdio.h>

int  serverPort = 0; //Port des Servers (ServerSocket) 
char terminalId[4] = "99";
char satzKennung = 'X';
#include <ESP8266WiFi.h>
#ifdef WEBSERVER
  //#include <WiFiClient.h>
  //#include <ESP8266WebServer.h>
  void handleRoot();              // function prototypes for HTTP handlers
  void handleLogin();
  void handleCommand();
  String webCommand = "";
  String webResult = "";
  // allows you to set the realm of authentication Default:"Login Required"
  const char* www_realm = "Custom Auth Realm";
  // the Content of the HTML response in case of Unautherized Access Default:empty
  String authFailResponse = "Authentication Failed";
  
  const char* www_username = "admin";
#endif

#include <EEPROM.h>
#include <LittleFS.h>

#include "common.h"
#include "myFS.h"
//#include "log.h"
#include "ntp.h"

int z = 0;                   //Aktuelle EEPROM-Adresse zum lesen
#include "Setup.h"

#ifdef SPITEST
  #include <SPI.h>
  #ifdef SPI_RFID
    #include <MFRC522.h>
  #endif
#endif

#ifdef IICTEST
  # include <Wire.h>             
  #ifdef IIC_KEYPAD
    #include <PCF8574.h>
    PCF8574 pcf8574(IO_I2C_ADDRESS); // default: 0x20
  # endif
  #ifdef IIC_RTC
    #include "RTClib.h"             //https://github.com/adafruit/RTClib
    RTC_DS3231 RTC;
  #endif
  #define BACKLIGHT_SECONDS 5
  #define BACKLIGHT_KEY (BACKLIGHT_SECONDS + 1)     // must be greater as BACKLIGHT_SECONDS
  int myBacklightTimer=BACKLIGHT_KEY;
  #ifdef IIC_DISPLAY
    #include <LiquidCrystal_I2C.h>
    LiquidCrystal_I2C lcd(DISPLAY_I2C_ADDRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address 0x27
  #endif
#endif

// Creat a set of new characters
const uint8_t charBitmap[][8] = {
   { 0b01010, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111, 0b00000 }, // ä
   { 0b01010, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01111, 0b00000 }, // ö
   { 0b01010, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101, 0b00000 }, // ü
   { 0b10001, 0b00100, 0b01010, 0b10001, 0b11111, 0b10001, 0b10001, 0b00000 }, // Ä
   { 0b10001, 0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000 }, // Ö
   { 0b10001, 0b00000, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110, 0b00000 }, // Ü
   { 0b01100, 0b10010, 0b10010, 0b11110, 0b10001, 0b10001, 0b11110, 0b10000 }, // ß
   { 0b00100, 0b00100, 0b00100, 0b00100, 0b10101, 0b01110, 0b00100, 0b00000 }, // ↓
   { 0b00100, 0b00100, 0b01110, 0b10101, 0b00100, 0b00100, 0b00100, 0b00000 }  // ↑
};

// esp8266 RFID-RC522
#ifdef SPITEST
  #define SS_PIN          15         // SDA    D8 (GPIO15)
                                     // CLK    D5
                                     // MOSI   D7
                                     // MISO   D6
                                     // IRQ    0 , war D1 -> D3 ?
                                     // GND
  #define RST_PIN         0         // RST    D0 (GPIO16) auf gleicher Reihe wie D5-D8, war D3(GPIO0), Test mit UNUSED_PIN keine Funktion
                                     // 3.3
#endif

// esp8266 I2C
#ifdef IICTEST
  #define PIN_SDA D2                // SDA    D2
  #define PIN_SCK D1                // SCK    D1 war D4 
#endif

int satzNummer = 9999;
char satzArt[3] = "FO";

//WiFi
int WifiCounter = 0; // connect 1 .. 30
int WifiFails = 0;   // 
unsigned long WifiNext = 0;   // paused
unsigned long WifiConnected = 0;

//Time
int NTPCounter = 0; // connect 1 .. 30
int NTPFails = 0;   // 
unsigned long NTPNext = 0;   // paused
unsigned long myTime = 0, myTime10 = 0, myTime600 = 0, myTimeId = 0;
unsigned long myTimeFromTics = 0;

//Display
#define LEER "                    "
char message[4][21] = {LEER, LEER, LEER, "                xxxx"};
char messageState[7] = "      ";
const byte message3   = 13;
const byte messageBL  = 14;
const byte messageUPL = 15;
const byte messageONL = 16;
const byte messageRTC = 17;
const byte messageNTP = 18;
const byte messageWIFI = 19;
char messageLater[3][21] = {LEER, LEER, LEER};
unsigned long messageWait[3] = {0, 0, 0};

//Keypad
char keypad[21] = LEER;
byte keypadPos  = 0;
bool keypadUnlocked = false;

//send and replay
WiFiClient client; 
ESP8266WebServer  httpserver(80); 
bool ServerOk = false;
String header;
char data[80];
char dataReturn[42];
int offlineCount = 0;
int offlineSend = 0;

//RFC
unsigned long chipID = 1;
unsigned long chipIDhex = 1;
int chipStatusNotOk = 0;
unsigned long chipIDPrev = 0;
boolean chipFirstLoop = true; // only for tests in loop()
// MFRC522-Instanz erstellen
#ifdef SPI_RFID
  MFRC522 mfrc522(SS_PIN, RST_PIN);
#endif
char eingabe[30] = "";
byte eingabePos = 0;
char version[2] = "0";

String Temp = "";

// Display
void displayByte(char myChar){
#ifdef IIC_DISPLAY
  if (DISPLAYok){
    switch (myChar) {
      case 132 /*ä*/ : lcd.write(byte(0)); break;
      case 148 /*ö*/ : lcd.write(byte(1)); break;
      case 129 /*ü*/ : lcd.write(byte(2)); break;
      case 142 /*Ä*/ : lcd.write(byte(3)); break;
      case 153 /*Ö*/ : lcd.write(byte(4)); break;
      case 154 /*Ü*/ : lcd.write(byte(5)); break;
      case 225 /*ß*/ : lcd.write(byte(6)); break;
      case  25 /*↓*/ : lcd.write(byte(7)); break;
      case  24 /*↑*/ : lcd.write(byte(8)); break;
      default  : lcd.print(myChar); break;
    }
  }
#endif
}

void displayZeile(int row, char* rowMessage){
  #ifdef IIC_DISPLAY
    if (DISPLAYok){
    bool ende = false;
    byte rowEnd = 0;
    
    rowEnd = (row == 3 ? message3 : 20);
    lcd.setCursor ( 0, row );        // go to the next line
    for (byte j = 0; j < rowEnd; j++) {
      if (rowMessage[j] == 0)
        ende = true;
      if (ende)
        lcd.print(' ');
      else
        displayByte(rowMessage[j]);
      //Serial.print((int)message[row][j]);Serial.println(message[row][j]);
    }
  }
  #endif
}

void displayText(int row = 0, char* rowMessage = &message[0][0], int rowWait = 0) {
  switch (row) {
    case 1 : 
    case 2 : 
    case 3 : 
      if (messageWait[row-1]>0 && rowWait == 0) {
        memcpy(messageLater[row-1], rowMessage, 21);
      } else {
        memcpy(message[row], rowMessage,  21);
        //message[row][21] = 0;
      }
      if (rowWait > 0){
        messageWait[row-1] = myTime + rowWait;
        memcpy(messageLater[row-1], rowMessage, 21);
      }
      break;
  }
  if (row < 4){
    displayZeile(row, message[row]);
  }
  
  for (int i=0;i<3;i++){
    if (messageWait[i]>0 && myTime >= messageWait[i]){
      memcpy(message[i+1], messageLater[i], 21);
      messageWait[i] = 0;
      displayZeile(i+1, message[i+1]);
    }
  }
  /*
  if (row > 0 && row < 4){
    for (int i=0; i<4;i++){
      if (row == i){
        Serial.print(message[i]);
        if (i>0)
          Serial.print(messageLater[i-1]);
      } else {
        Serial.print("                    ");
        if (i>0)
          Serial.print("                    ");
      }
      Serial.print(";");
    }
    Serial.println("");
  }
  */
}

void displayChar(int pos, int row, char myChar){
  #ifdef IIC_DISPLAY
    if (DISPLAYok){
      lcd.setCursor ( pos, row );        // go to the next line
      if (row==3) message[3][pos]=myChar;
      displayByte(myChar);
    }
  #endif
  if (pos >=14 && pos <= 19)
  messageState[pos-14] = myChar;
}

void setBacklight(int set = 0) {
  #ifdef IIC_DISPLAY
    if (DISPLAYok){
      boolean isOn = (myBacklightTimer > 0);
      if (set == 0){ // loop per second, toggle if timer gets to 0
        if (myBacklightTimer > 0 && myBacklightTimer <= BACKLIGHT_SECONDS){
          myBacklightTimer--;
        }
      } else if (set == BACKLIGHT_KEY){ // toggle with "*"
        if (myBacklightTimer > 0){
          myBacklightTimer = 0;
        } else {
          myBacklightTimer = BACKLIGHT_KEY;
        }
      } else if (set > myBacklightTimer){
        myBacklightTimer = set;
      } 
      boolean toggle = (isOn != (myBacklightTimer > 0));
      if (toggle){
        lcd.setBacklight(!isOn); 
      }
    }
  #endif
}

bool connectServer(){
  if (!WifiConnected){
    return false;
  }
  client.setTimeout(500);
  if (!client.connect(serverHost, serverPort)) { 
    return false; 
  } 
  //Serial.println(); 
  //Serial.print("Verbunden mit "); 
  //Serial.println(serverHost); 
  displayChar(messageONL, 3, 'O');
  return true;
}

bool closeServer(){
  /*Verbindung zum Server schliessen*/ 
  //Serial.println("Verbindung schliessen"); 
  client.flush(); 
  client.stop(); 
  //Serial.println("Verbindung geschlossen");  
  return true;
}

void testServer(bool command = false){
  if (!ServerOk || command){
    if (connectServer()){
      if (command){
        testServerCommand();
      }
      delay(100);
      ServerOk = closeServer();
    }
  }
}

void toDo(char* eingabe, byte eingabePos){
  byte pos;
  int ziffer=0;
  //eingabePos = eingabe.length();
  Serial.println(eingabe);
  webResult = String(eingabe);
  switch (eingabe[0]){
    case 'a' : for (pos=0; pos<=eingabePos; pos++) ssid[pos] = eingabe[pos+1]; break;
    case 'b' : for (pos=0; pos<=eingabePos; pos++) passwort[pos] = eingabe[pos+1]; break;
    case 'c' : for (pos=0; pos<=eingabePos; pos++) UpdateServer[pos] = eingabe[pos+1]; break;
    case 'd' : for (pos=0; pos<=eingabePos; pos++) timeserver[pos] = eingabe[pos+1]; break;
    case 'e' : for (pos=0; pos<=eingabePos; pos++) device_name[pos] = eingabe[pos+1]; break;
    case 'f' : for (pos=0; pos<=eingabePos; pos++) terminalId[pos] = eingabe[pos+1]; break;
    case 'g' : for (pos=0; pos<=eingabePos; pos++) serverHost[pos] = eingabe[pos+1]; break;
    case 'h' : serverPort = 0; 
               for (pos=1; pos<=eingabePos; pos++) {
                 ziffer = eingabe[pos];
                 ziffer = ziffer - 48;
                 if (ziffer >= 0 and ziffer <= 9)
                   serverPort = serverPort * 10 + ziffer; 
               }
               break;
    case 'i' : for (pos=0; pos<=eingabePos; pos++) keypass[pos] = eingabe[pos+1]; break;
    case 'j' : for (pos=0; pos<=eingabePos; pos++) www_password[pos] = eingabe[pos+1]; break;
    
    case 'r' : configRead(); break;
    case 'p' : configPrint(); break;
    case 'w' : configWrite(); break;
    
    case 's' : testSPI(); break;
    case 't' : testIIC(); break;
    case 'u' : webResult = handleDirList("/",3); Serial.println(webResult); break;
    
    case 'o' : testServer(true); break;
    case 'v' : webResult = String(hardware) +";"+String(versionNr); Serial.println(webResult); break;
    case 'x' : Serial.print("OfflineCount: ");Serial.print(offlineCount);Serial.print(", OfflineSend: ");Serial.println(offlineSend); break;
    case 'z' : ESP.restart(); break;
    case '0' : ota(); break;
    case '*' : setBacklight(BACKLIGHT_KEY); break;
  }
}

void testServerCommand(){
  snprintf(data, 80, "R_%2sJ%4sC__CC", terminalId, hardware);
  client.println(data); 
  delay(100); 
  /*Echo vom Server lesen und eventuellen Befehl ausführen*/ 
  String line = client.readStringUntil('\n'); 
  if (strncmp(line.c_str(),"update", 6) == 0) {
    ota();
  } else {
    strcpy(eingabe, line.c_str());
    toDo(eingabe, line.length());
    Serial.print("command?");
    Serial.print(line);
    Serial.println("?command");
  }
}

bool sendToServer(bool onlyOffline = false){
  bool myConnected = true;
  int offlineLine = 0;
  if (connectServer()){
    // Daten im Offline-Speicher?
    File fin = LittleFS.open("/data/01", "r");
    if (fin) {
      while (myConnected && offlineCount > offlineSend && fin.available()){
        //fin.seek((offlineSend-1)*51, SeekSet);
        //if(fin.available()){
        // '\n' is not included in the returned string, but the last char '\r' is
        String line=fin.readStringUntil('\n');
        offlineLine++;
        if (offlineLine > offlineSend) {
          // nur neue, nicht gesendete übertragen
          displayChar(messageUPL, 3, '>');
          Serial.print("SE-Line: ");
          Serial.println(line);
          client.println(line); 
          delay(100); 
          /*Echo vom Server lesen und verwerfen, da alte Daten*/ 
          displayChar(messageUPL, 3, '<');
          line = client.readStringUntil('\n'); 
          if (line.length() == 0){
            myConnected = false;
          } else {
            line.toCharArray(dataReturn, 42);
            displayText(3, dataReturn+20);
            offlineSend++;
            snprintf(data, message3, "Offline: %d          ", (offlineCount - offlineSend) ) ;
            displayText(3, data);
            File fout = LittleFS.open("/data/offline", "a");
            if (fout) {
              fout.write(offlineSend);
              //int offlineData = fout.size();
              fout.close();
              //Serial.print("SE-Size: ");
              //Serial.println(offlineData);
            } else {
              //Serial.println("file offline open failed");
            }
          }
        }
      }
      fin.close();
    }
    else {
      //Serial.println("file offline open failed");
    }
    
    if (offlineLine == offlineSend && offlineLine > 0) {
      LittleFS.remove("/data/offline");
      LittleFS.remove("/data/01");
      offlineCount = 0;
      offlineSend = 0;
      displayText(3, (char *)LEER);
    }
    
    if (!onlyOffline){
      displayChar(messageUPL, 3, '>');
      //Serial.print("Nachricht an Server senden: "); 
      Serial.println(data); 
      client.println(data); 
      delay(100); 
     
      /*Echo vom Server lesen und ausgeben*/ 
      displayChar(messageUPL, 3, '<');
      String line = client.readStringUntil('\n'); 
      if (line.length() == 0){
        myConnected = false;
      } else {
        line.toCharArray(dataReturn, 42);
        //Serial.print("Online:");
        //Serial.print(line); 
        //Serial.println(); 
        //Serial.print("Online:");
        //Serial.print(data); 
        //Serial.println(); 
        
        displayText(2, dataReturn, 4);
        if (line.length() > 22) {
          displayText(3, dataReturn+20,4);
        } else {
          displayText(3, (char *)"");
        }
      }
      displayChar(messageUPL, 3, '#');
    }
    ServerOk = closeServer();
  } else {
    ServerOk = false;
  }
  return ServerOk;
}

void sendAndReplay(unsigned long id) {
    /*
  1 dppz.terminal.id    99
  2 dppz.satz.nr        0000
  3 dppz.satz.kennung   X
  4 dppz.satz.art       F0 (Automatik), sonst KO, GE, DG, SO
  5 dppz.karten.nr      50 (Bielemeier)
  R_90J2222X__FO0000000246787666520220706112409_00000000001515270
*/
    //snprintf(data, 80, "%s %s %10d 4d%2d%2d%2d%2d%2d       ", message[3][messageWIFI], terminal, id, year(), month(), day(),hour(), minute(), second()) ;
    //R_11J22223__44_________5555555566666666777777____
    //snprintf(data, 80, "R%3sJ%4d%c__%2s_________%08d%04d%02d%02d%02d%02d%02d____", message[3][messageWIFI], terminalId, satzNummer, satzKennung, satzArt, id, year(), month(), day(),hour(), minute(), second()) ;
    //2020-03-10 snprintf(data, 80, "R_%2sJ2222%c__%2s_____%012lu%04d%02d%02d%02d%02d%02d____", terminalId, satzKennung, satzArt, id, year(), month(), day(),hour(), minute(), second());
    snprintf(data, 80, "R_%2sJ2222%c__%2s%017lu%04d%02d%02d%02d%02d%02d_%017lu", terminalId, satzKennung, satzArt, id, year(), month(), day(),hour(), minute(), second(), chipID);

    //sendToServer();
    displayChar(messageUPL, 3, '>');
    if (!sendToServer()) {
      displayChar(messageUPL, 3, '#');
      displayChar(messageONL, 3, '-');
      offlineCount++;
      data[0] = '[';
      //save to File and send later
      File f = LittleFS.open("/data/01", "a");
      if (f) {
        f.println(data);
        //int offlineData = f.size();
        f.close();
        //Serial.print("01-Size: ");
        //Serial.print(offlineData);
        //Serial.print(": ");
        //Serial.println(data);
      } else {
        //Serial.println("file 01 open failed");
        displayText(1, (char*)"Fehler Offline");
      }
      /////////////////////////////////////////dataWrite();
      //Serial.print("Offline:");
      //Serial.println(data);
      snprintf(data, 21, "gelesen: %ld         ", id) ;
      displayText(2, data, 4);
      snprintf(data, message3, "Offline: %d          ", (offlineCount - offlineSend) ) ;
      displayText(3, data);
    }
  displayChar(messageUPL, 3, '#');
}

# ifdef IICTEST
void testIIC()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      if (address == IO_I2C_ADDRESS)      IOok  = 1;
      if (address == RTC_I2C_ADDRESS)     RTCok = 1;
      if (address == DISPLAY_I2C_ADDRESS) DISPLAYok = 1;
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Error ");
      Serial.print(error);
      Serial.print(" at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  hardware[3] = '0';
  if (nDevices == 0){
    webResult = "No I2C devices found";
    Serial.println(webResult);
  } else {
    webResult = "";
    #ifdef IIC_KEYPAD
      if (IOok) { 
        webResult += "IO ok - ";
        hardware[3] = hardware[3] + 1; 
      }
    #endif
    #ifdef IIC_RTC
      if (RTCok) { 
        webResult += "RTC ok - ";
        hardware[3] = hardware[3] + 2; 
      }
    #endif
    #ifdef IIC_DISPLAY
      if (DISPLAYok) { 
        webResult += "DISPLAY ok - ";
        hardware[3] = hardware[3] + 4; 
      }
    #endif
    Serial.println(webResult);
  }
}
#endif

void getRTC(){
  #ifdef IIC_RTC
    if (RTCok) {
      RTCTime = RTC.now().unixtime();
      nowTime = now();
      if(RTCTime > nowTime ? RTCTime - nowTime > 5 : nowTime - RTCTime > 5) {
        //LogSchreibenNow("falsche Zeit");
        setTime(RTCTime);
        //LogSchreiben("RTC: Zeit gesetzt");
      }
      displayChar(messageRTC, 3, 'R');
    } else {
      displayChar(messageRTC, 3, '-');
    }
  #endif
}

void Zeit_Einstellen(){
  if (NTPok && NTPNext < myTime) {
    NTPok      = false;
    NTPCounter = 0;
    NTPFails   = 0;
  }
  if (!NTPok && NTPNext < myTime) {
    if (WifiConnected > 0) {
      NTPCounter++;
      if (NTPCounter == 1){
        sendNTP();
      } else  if (NTPCounter >= 30){
        NTPFails++;
        NTPCounter = 0;
        if (NTPFails > 4){
          // 30 Min. Pause, dann neu testen
          NTPNext = myTime + 30*60;
        }
      } else {
        NTPTime = GetNTP();
        if (NTPTime > 0){
          // ok, ab jetzt alle 6 Stunden NTP prüfen
          NTPok = true;
          NTPNext = myTime + 6*60*60;
          displayChar(messageNTP, 3, 'T');
          setTime(NTPTime);
          RTCTime = NTPTime;
          #ifdef IIC_RTC
            if (RTCok){
              RTCTime = RTC.now().unixtime();
            }
          #endif
          bool timediff = (RTCTime > NTPTime ? RTCTime - NTPTime > 5 : NTPTime - RTCTime > 5);
          if (timediff){
            Temp = PrintDate(RTCTime) + "   " + PrintTime(RTCTime) + "   falsche RTC-Zeit";
            Serial.println( Temp );
            //LogSchreiben(Temp);
            //Serial.println( Temp );
            RTCSync = NTPTime;
            #ifdef IIC_RTC
              RTC.adjust(DateTime(year(NTPTime), month(NTPTime), day(NTPTime), hour(NTPTime), minute(NTPTime), second(NTPTime)));
              RTCTime = RTC.now().unixtime();
            #endif
            Temp = PrintDate(RTCTime) + "   " + PrintTime(RTCTime) + "   richtige RTC-Zeit";
            Serial.println( Temp );
          }
          /*
          if(abs(NTPTime - now()) > 5) {
            //LogSchreibenNow("falsche Zeit");
            //Serial.println( Temp );
            //LogSchreiben("NTP: Zeit gesetzt");
            //Serial.println( Temp );
          }
          */
        }
      }
    }
  }
  getRTC();
  //displayText(3, message[3]);
}

// Function to connect WiFi
void connectWifi() {
  if (WiFi.status() != WL_CONNECTED && WifiNext < myTime)
  {
    WifiConnected = 0;
    WifiCounter++;
    if (WifiCounter == 1){
      WiFi.setAutoConnect(false);
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);                                            // Disable AP mode
      WiFi.begin(ssid, passwort);
      displayText(1, (char*)"Connecting to:      ");
      snprintf(data, 21, "%s                  ", ssid);
      displayText(2, data);
    } else if (WifiCounter >= 30){
      WifiFails++;
      WifiCounter = 0;
      if (WifiFails > 4){
        WifiNext = myTime + 30*60;
      }
    }
    snprintf(data, message3+2, "%d  %d                  ", WifiCounter, WifiFails) ;
    displayText(3, data);
    displayChar(messageWIFI, 3, '-');
    Serial.println(data);
  } else {
    // connected
    if (WifiConnected == 0){
      //snprintf(message[1], 21, "Connected to:       ") ;
      //snprintf(message[2], 21, "%s, %s              ", ssid, "TODO") ;
      displayChar(messageWIFI, 3, 'W');
      Serial.println(WiFi.localIP());
      WifiConnected = now();
      // now show hint
      chipID = 1;
      chipIDhex = 1;
    }
  }
}

#ifdef IIC_KEYPAD
  #define keymapRows 4
  #define keymapCols 4
  #define keymapClick true
  boolean keymapPause = true;
  //uint8_t pcf8574Last[keymapCols]= {0};
  const char keymap[keymapRows][keymapCols + 1] =
  {
    "123A",
    "456B",
    "789C",
    "*0#D"
  };

  void keypadloop(){
    uint8_t i,j = 0;
    uint8_t test = 0;
    uint8_t test2 = 0;
    uint8_t keymapX = 0;
    if (IOok){
      test = pcf8574.read8();
      if (test != 0xF0){
        //Serial.print("keypad ");Serial.println(test, BIN);
        if (keymapPause){
          //pcf8574Last[i] = 0;
          keymapPause = false;
          
          for (i = 0; i < keymapCols; i++){
            pcf8574.write8(~(0x08 >> i)); // links nach rechts
            delay(3);
            test = ~(pcf8574.read8());
            //Serial.print("keypad2 ");Serial.println(test, BIN);
            if (test > 0x0F){
              for (j = 0; j < keymapRows; j++){
                test2 = (0x80 >> j);
                //Serial.print("keypad3 ");Serial.println(test2, BIN);
                keymapX = test & test2;
                if (keymapX){
                  //Serial.print("keypad  ");Serial.println(keymap[j][i]);
                  if (keymap[j][i] == '*'){
                    setBacklight(BACKLIGHT_KEY);
                  } else {
                    setBacklight(BACKLIGHT_SECONDS);
                    switch (keymap[j][i]){
                      case '#' : 
                        keypadPos = 0;
                        keypad[0] = 0;
                        displayText(3, (char *)"#"); 
                        keypadUnlocked = false;
                        break;
                      case 'A' : snprintf(satzArt, 3, "KO"); displayText(3, (char*)"Kommen"); break;
                      case 'B' : snprintf(satzArt, 3, "GE"); displayText(3, (char*)"Gehen"); break;
                      case 'C' : snprintf(satzArt, 3, "KR"); displayText(3, (char*)"Gehen Krank"); break;
                      case 'D' : snprintf(satzArt, 3, "DG"); displayText(3, (char*)"Dienstgang"); break;
                      default  : {
                        if (!keypadUnlocked) {
                          if (keypadPos == 21){
                            keypadPos = 0;
                          }
                          keypad[keypadPos] = keymap[j][i]; 
                          if (keypadPos < 2){
                            satzArt[keypadPos] = keymap[j][i];
                            satzArt[keypadPos+1] = 0;
                          }
                          keypadPos++;
                          keypad[keypadPos] = 0; 
                          
                          //Serial.print("keypad  ");
                          //Serial.print(keypadPos);
                          //Serial.println(keymap[j][i]);
    
                          if (strcmp(keypad, keypass) != 0) {
                            displayText(3, keypad);  
                          } else {
                            keypadUnlocked = true;
                            keypadPos = 0;
                            keypad[0] = 0;
                            displayText(3, (char*)"unlocked", 4);
                          }
                        } else {
                          if (keymap[j][i] == '0') ota();
                        }
                        break;
                      }
                    }
                  }
                  //displayChar(0,3, keymap[j][i]);
                  chipIDPrev = 0;
                }
              }
            }
          }
        }
        pcf8574.write8(0xF0); // warten, ob irgendeine Taste gedrückt wird
      } else {
        keymapPause = true;
      }
    }
  }
#endif

void configRead() {
  //Serial.println("configRead");
  z = 0;
  EEPROM.begin(512);
  LeseEeprom(version, 2);                 //  0
  LeseEeprom(ssid, sizeof(ssid));         //  2
  LeseEeprom(passwort, sizeof(passwort)); // 
  LeseEeprom(UpdateServer, LOGINLAENGE);
  LeseEeprom(timeserver, LOGINLAENGE);
  LeseEeprom(device_name, LOGINLAENGE);
  LeseEeprom(terminalId, 4);
  LeseEeprom(serverHost, LOGINLAENGE);
  serverPort = LeseEeprom();
  LeseEeprom(keypass, LOGINLAENGE);
  if (version[0] <= '2') {
    myBacklightTimer = BACKLIGHT_KEY;
  } else {
    myBacklightTimer = LeseEeprom();
  }
  if (version[0] > '3') {
    LeseEeprom(www_password, sizeof(www_password)); // 
  }
  LeseEepromCheck();                // V2 Pos 266
  EEPROM.end();
}

void configWrite() {
  z = 0;
  EEPROM.begin(512);
  SchreibeEeprom(versionNeu, 2 );
  SchreibeEeprom(ssid, sizeof(ssid));
  SchreibeEeprom(passwort, sizeof(passwort));
  SchreibeEeprom(UpdateServer, LOGINLAENGE);
  SchreibeEeprom(timeserver, LOGINLAENGE);
  SchreibeEeprom(device_name, LOGINLAENGE);
  SchreibeEeprom(terminalId, 4);
  SchreibeEeprom(serverHost, LOGINLAENGE);
  SchreibeEeprom(serverPort);
  SchreibeEeprom(keypass, LOGINLAENGE);
  SchreibeEeprom(myBacklightTimer);
  SchreibeEeprom(www_password, LOGINLAENGE);
  SchreibeEepromCheck();
  EEPROM.commit();
  EEPROM.end();
}

void configPrint() {
  Serial.print("Version "); Serial.println(version);
  Serial.print("a "); Serial.println(ssid);
  Serial.print("b "); Serial.println(passwort);
  Serial.print("c "); Serial.println(UpdateServer);
  Serial.print("d "); Serial.println(timeserver);
  Serial.print("e "); Serial.println(device_name);
  Serial.print("f "); Serial.println(terminalId);
  Serial.print("g "); Serial.println(serverHost);
  Serial.print("h "); Serial.println(serverPort);
  Serial.print("i "); Serial.println(keypass);
  Serial.print("j "); Serial.println(www_password);
}

void Serial_Task() {
  while (Serial.available() > 0)  
  { // Eingabe im Seriellen Monitor lesen
    setBacklight(BACKLIGHT_SECONDS);
    char Zeichen = Serial.read();    
    byte pos = 0;
    int ziffer = 0;
    if (Zeichen == '\n') 
    { // Enter/Senden gedrückt
      eingabe[eingabePos] = 0;
      toDo(eingabe, eingabePos-1);
      eingabePos = 0; 
    } else {
      eingabe[eingabePos++] = Zeichen;
    }
  }
}

void ota(){
  displayText(2, (char*)"Suche Update ...");
  WiFiClient wifiClient;
#ifdef ESP32
  t_httpUpdate_return ret = httpUpdate.update(wifiClient, UpdateServer, 80, "/esp8266/ota.php", mVersionNr);
#else      
  t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, UpdateServer, 80, "/esp8266/ota.php", versionNr);
  //t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, UpdateServer, 80, "/esp8266/ota.php", "V00-03-01.trt.d1_mini");
#endif
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      DEBUG_OUTPUT.println("[update] Update failed: "); 
      displayText(2, (char*)"Update fehlgeschlagen", 4);
      break;
    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_OUTPUT.println("[update] Update no Update.");
      displayText(2, (char*)"Version ist aktuell", 4);
      break;
    case HTTP_UPDATE_OK:
      DEBUG_OUTPUT.println("[update] Update ok."); // may not called we reboot the ESP
      displayText(2, (char*)"Update erfolgreich", 4);
      break;
    default:
      DEBUG_OUTPUT.println("update] Update default?");
      displayText(2, (char*)"Update default?", 4);
      break;
  }
}

void testSPI(){
  #ifdef SPI_RFID
    RFIDok = (mfrc522.PCD_DumpVersionToSerial() > 0); 
    hardware[2] = RFIDok ? '1' : '0';
  #endif
}

void handleRoot() {
  String message = "<!doctype html><html lang=\"de\"><head><meta charset=\"utf-8\"></head><body>";
         message += String(keypadUnlocked ? "un" : "")+"locked - Befehl: "+webCommand+" - Antwort: "+webResult+"<br />";
         message += "<form action=\"/command\" method=\"POST\"><input type=\"password\" name=\"password\" placeholder=\"Password\"><br />";
         message += "<input type=\"text\" name=\"command\" placeholder=\"Command\"><input type=\"submit\" value=\"command\"></form><br />";
         message += "<form action=\"/list\" method=\"POST\"><input type=\"text\" name=\"dir\" value=\"/\"><br /><input type=\"submit\" value=\"list\"></form><br />";
         message += "XX</body></html>";
  httpserver.send(200, "text/html", message );   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleLogin() {                          // If a POST request is made to URI /LED
  if( httpserver.hasArg("password") ) { // If both the username and the password are correct
    keypadUnlocked = (strcmp(httpserver.arg("password").c_str(), keypass) == 0);
    //displayText(3, (char*)"unlocked", 4);
  }
  httpserver.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  httpserver.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleCommand() {                          // If a POST request is made to URI /LED
  if( httpserver.hasArg("password") ) { // If both the username and the password are correct
    keypadUnlocked = (strcmp(httpserver.arg("password").c_str(), keypass) == 0);
    //displayText(3, (char*)"unlocked", 4);
    if( keypadUnlocked && httpserver.hasArg("command") ) { // If both the username and the password are correct
      webCommand = httpserver.arg("command");
      int webLength = webCommand.length() < 30 ? webCommand.length() : 30;
      webCommand.toCharArray(eingabe, webLength+1 );
      Serial.print("ToDo: ");Serial.print(eingabe);Serial.print("-");Serial.println(webLength);
      toDo(eingabe , webLength);
      //displayText(3, (char*)"unlocked", 4);
      String output = "[";
      output += "{\"lock\":\"";
      output += String(keypadUnlocked);
      output += "\",\"command\":\"";
      output += webCommand;
      output += "\",\"result\":\"";
      output += webResult;
      output += "\",\"display0\":\"";
      output += message[0];
      output += "\",\"display1\":\"";
      output += message[1];
      output += "\",\"display2\":\"";
      output += message[2];
      output += "\",\"display3\":\"";
      output += message[3];
      output += "\",\"display4\":\"";
      output += messageState;
      output += "\"}]";
      httpserver.send(200, "text/json", output);
      return;
    }
  }
  httpserver.send(200, "text/json", "[{\"command\":\"leere Anfrage oder falsches Passwort\"}]");
  //httpserver.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
  //httpserver.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

byte is_authentified(){
  return 1;
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  configRead();

  // SPI-Bus initialisieren
  #ifdef SPITEST
    SPI.begin();
    #ifdef SPI_RFID
      // MFRC522 initialisieren
      mfrc522.PCD_Init();
      // Details vom MFRC522 RFID READER / WRITER ausgeben
      //Kurze Pause nach dem Initialisieren   
      //hierdurch Absturz? 
      delay(10);
      testSPI();
    #endif
  #endif

# ifdef IICTEST
    Wire.begin(PIN_SDA, PIN_SCK);
    testIIC();
    #ifdef IIC_DISPLAY
      if (DISPLAYok) {
        //oledSplash();
        int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));
        lcd.begin(20,4);               // initialize the lcd 
        for ( int i = 0; i < charBitmapSize; i++ )
        {
          lcd.createChar ( i, (uint8_t *)charBitmap[i] );
        }
        lcd.home ();                   // go home
        displayText(1, (char*)"Display ok");
      }
    #endif
  #endif

  LittleFS.begin();
  File fin = LittleFS.open("/data/01", "r");
  if (fin) {
    offlineCount = fin.size()/64;
    fin.close();
  }
  File fout = LittleFS.open("/data/offline", "r");
  if (fout) {
    offlineSend = fout.size();
    fout.close();
  }
  FSInfo fs_info;
  LittleFS.info(fs_info);
  Serial.println("LittleFS total " + String(fs_info.totalBytes)+ ", used " + String(fs_info.usedBytes));

  //httpserver.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  httpserver.onNotFound(handleFile);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  httpserver.on("/login", HTTP_POST, handleLogin);
  httpserver.on("/command", HTTP_POST, handleCommand);
  httpserver.on("/list", HTTP_POST, handleFileList);
  httpserver.on("/list", HTTP_GET, handleFileList);
 
  httpserver.on("/delete", handleFileDelete);
  httpserver.on("/upload", HTTP_GET, []() {
    if (!httpserver.authenticate(www_username, www_password)) {
      return httpserver.requestAuthentication();
    }
    String serverIndex = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>";
    httpserver.sendHeader("Connection", "close");
    httpserver.sendHeader("Access-Control-Allow-Origin", "*");
    httpserver.send(200, "text/html", serverIndex);
  });
  httpserver.on("/upload", HTTP_POST, handleUpload, handleFileUpload);
  
  httpserver.begin();

  displayChar(messageNTP, 3, '-');
  displayChar(messageONL, 3, '-');
  getRTC();
}

void loop() {
  unsigned long myNow = now();
  unsigned long myNow10 = 0;
  unsigned long myNow600 = 0;

  if (chipIDhex > 0) {
    displayText(1, (char*)"Karte bitte ...") ;
    displayText(2, (char*)"");
    displayText(3, (char*)"");
    //CardID resetten
    chipID = 0;
    chipIDhex = 0;
    chipStatusNotOk = 0;
    snprintf(satzArt, 3, "FO");
    keypadPos = 0;
    keypad[0] = 0;
  }
  if (myTime != myNow) {
    // loop per second
    myTime = myNow;
    myNow10  = myNow / 60; // Test von 10 Sek. auf 60 Sek
    myNow600 = myNow / 600;
    myTimeFromTics = millis() / 1000;
    snprintf(message[0], 21, "%02d.%02d.%04d %02d:%02d:%02d ", day(), month(), year(),hour(), minute(), second()) ;
    displayText();
    //WiFi check
    connectWifi();          // Try to connect WiFi aSync
    if (WifiConnected > 0 ){
      Zeit_Einstellen();
      if (myNow10 != myTime10){
        // loop per 10 seconds
        myTime10 = myNow10;
        if (offlineCount > offlineSend){
          sendToServer(true);
        } else {
          //testServer(true);//myNow600 != myTime600); // Test jedesmal Auftrag abfragen
        }
        if (myNow600 != myTime600){
          // loop per 10 minutes
          myTime600 = myNow600;
          testServer(true);
        }
      }
    }
    setBacklight();
  }
  #ifdef SPI_RFID
    // Sobald ein Chip aufgelegt wird startet diese Abfrage
      if (chipFirstLoop)
        displayChar(messageBL, 3, 'A');
      if (mfrc522.PICC_IsNewCardPresent()){
      delay(200);
      displayChar(messageBL, 3, 'B');
      MFRC522::StatusCode result =mfrc522.PICC_Select(&mfrc522.uid);
	    Serial.print("PICC-STATUS:");
	    Serial.println(result);
      setBacklight(BACKLIGHT_SECONDS);
      displayChar(messageBL, 3, result);
      if (result == MFRC522::STATUS_OK){
        // Hier wird die ID des Chips in die Variable chipID geladen
        chipID = 0;
        chipIDhex = 0;
        //Serial.print("chipID 0x");
        for (byte i = 0; i < mfrc522.uid.size; i++){
          byte myByte = mfrc522.uid.uidByte[i];
          //if (myByte < 16)
          //  Serial.print("0");
          //Serial.print(myByte,HEX);
          //BUG!!
          chipID=((chipID + myByte)*10);
          chipIDhex=chipIDhex*256 + myByte;
        }
        //Serial.print("/");
        //Serial.print(chipIDhex);
        //Serial.print("/bisher: ");
        //Serial.println(chipID);
    
        //... und anschließend ausgegeben wenn nicht doppelt innerhalb von 5 Sekunden
        if (chipIDhex > 0 and (chipIDhex != chipIDPrev || myNow > myTimeId + 4) ) {
          displayChar(messageBL, 3, 'C');
          myTimeId = myNow;
          chipIDPrev = chipIDhex;
          sendAndReplay(chipIDhex);
        }
        // Danach 0,3 Sekunde pausieren, um mehrfaches lesen /ausführen zu verhindern
        delay(300);
      } else {
        //Lesefehler
        if (chipStatusNotOk++ == 2)
        {
          //Serial.println("Lesefehler!");
          displayText(2, (char*)"Lesefehler!", 2);
          // Anzeige beim nächsten Loop zurücksetzen
          chipID = 1;
          chipIDhex = 1;
        }
      }
    }
  #endif

  #ifdef IIC_KEYPAD
    if (IOok)
      keypadloop();
  #endif
  
  Serial_Task();
  httpserver.handleClient();                    // Listen for HTTP requests from clients

  chipFirstLoop = false;
}
