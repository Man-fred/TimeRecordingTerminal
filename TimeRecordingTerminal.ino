// Basic MFRC522 RFID Reader Code by cooper @ my.makesmart.net
// Released under Creative Commons - CC by cooper@my.makesmart.net
#include "defines.h";

char ssid[32] = "\0";
char passwort[64] = "\0";
char serverHost[LOGINLAENGE] = "192.168.178.1"; //IP des Servers 
int  serverPort = 0; //Port des Servers (ServerSocket) 
char terminalId[4] = "99";
const char* satzKennung = "X";

#include <stdio.h>

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <LittleFS.h>

#include "common.h";
#include "myFS.h";
//#include "log.h"
#include "ntp.h"

int z = 0;                   //Aktuelle EEPROM-Adresse zum lesen
#include "Setup.h"

#include <SPI.h>
#include <MFRC522.h>

#ifdef IICTEST
# include <Wire.h>               //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
# ifdef IIC_KEYPAD
#   include <PCF8574.h>
    PCF8574 pcf8574(0x20);
    boolean pcf8574Active = true;
# endif
# ifdef IIC_RTC
#   include "RTClib.h"             //https://github.com/adafruit/RTClib
    RTC_DS3231 RTC;
# endif
# ifdef IIC_DISPLAY
#   include <LiquidCrystal_I2C.h>
//#   define BACKLIGHT_PIN     13
    bool myBacklight=true;
    LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#endif

// Creat a set of new characters
const uint8_t charBitmap[][8] = {
   { 0xc, 0x12, 0x12, 0xc, 0, 0, 0, 0 },
   { 0x6, 0x9, 0x9, 0x6, 0, 0, 0, 0 },
   { 0x0, 0x6, 0x9, 0x9, 0x6, 0, 0, 0x0 },
   { 0x0, 0xc, 0x12, 0x12, 0xc, 0, 0, 0x0 },
   { 0x0, 0x0, 0xc, 0x12, 0x12, 0xc, 0, 0x0 },
   { 0x0, 0x0, 0x6, 0x9, 0x9, 0x6, 0, 0x0 },
   { 0x0, 0x0, 0x0, 0x6, 0x9, 0x9, 0x6, 0x0 },
   { 0x0, 0x0, 0x0, 0xc, 0x12, 0x12, 0xc, 0x0 }
};

#endif
// esp8266 RFID-RC522
#define SS_PIN          15         // SDA    D8
                                   // CLK    D5
                                   // MOSI   D7
                                   // MISO   D6
                                   // IRQ    D1 -> trennen ?
                                   // GND
#define RST_PIN         0          // RST    D3
                                   // 3.3
// esp8266 I2C
#ifdef IICTEST
# define PIN_SDA D2                // SDA    D2
# define PIN_SCK D4                // SCK    D4 (eigentlich D1)
#endif

int satzNummer = 9999;
char satzArt[3] = "FO";

//WiFi
int WifiCounter = 0; // connect 1 .. 30
int WifiFails = 0;   // 
unsigned long WifiNext = 0;   // paused
unsigned long WifiConnected = 0;

//Time
unsigned long myTime = 0;
unsigned long myTimeFromTics = 0;

//Display
#define LEER "                    "
char message[4][21] = {LEER, LEER, LEER, "                xxxx"};
const byte message3 = 14;
const byte messageONL = 16;
const byte messageRTC = 17;
const byte messageNTP = 18;
const byte messageWIFI = 19;
char messageLater[3][21] = {LEER, LEER, LEER};
unsigned long messageWait[3] = {0, 0, 0};

//send and replay
char data[80];
char dataReturn[21];
int offlineCount = 0;
int offlineSend = 0;

//RFC
long chipID = 1;
// MFRC522-Instanz erstellen
MFRC522 mfrc522(SS_PIN, RST_PIN);

char eingabe[30] = "";
byte eingabePos = 0;
char version[2] = "0";

String Temp = "";

// Display
void displayText(int row = 0, char* rowMessage = &message[0][0], int rowWait = 0) {
  switch (row) {
    case 1 : 
    case 2 : 
      if (messageWait[row-1]>0 && rowWait == 0)
        memcpy(messageLater[row-1], rowMessage, 21);
      else
        memcpy(message[row], rowMessage, 21);
      if (rowWait > 0){
        messageWait[row-1] = myTime + rowWait;
        memcpy(messageLater[row-1], rowMessage, 21);
      }
      break;
    case 3 : 
      lcd.setCursor ( 0, 3 );
      for (byte i=0; i<message3; i++){
        //memcpy(message[row], rowMessage, message3);
        message[3][i] = (rowMessage[i] != 0 ? rowMessage[i] : ' ') ;
        lcd.print (message[3][i]);
      }
      /*message[3][message3] = ' ';
      message[3][messageONL] = myConnected ? 'O' : '-';
      message[3][messageRTC] = myConnected ? 'R' : '-';
      message[3][messageNTP] = myConnected ? 'N' : '-';
      message[3][messageWIFI] = myConnected ? 'W' : '-';*/
      break;
  }
  #ifdef IIC_DISPLAY
    if (row < 3){
      lcd.setCursor ( 0, row );        // go to the next line
      lcd.print (message[row]);
    }
  #endif
  
  for (int i=0;i<2;i++){
    if (messageWait[i]>0 && myTime >= messageWait[i]){
      memcpy(message[i+1], messageLater[i],21);
      messageWait[i] = 0;
      #ifdef IIC_DISPLAY
        lcd.setCursor ( 0, i+1 );        // go to the next line
        lcd.print (message[i+1]);
      #endif
    }
  }
  if (row > 0 && row < 3){
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
}

void displayChar(int pos, int row, char myChar){
  #ifdef IIC_DISPLAY
    lcd.setCursor ( pos, row );        // go to the next line
    lcd.print (myChar);
  #endif
}

bool sendToServer()
{
  bool myConnected = false;
  if (!WifiConnected){
    return false;
  }

  WiFiClient client; 
   
  if (!client.connect(serverHost, serverPort)) { 
    //Serial.print("X"); 
    return false; 
  } 

  myConnected = true;
  Serial.println(); 
  Serial.print("Verbunden mit "); 
  Serial.println(serverHost); 
 
  //Serial.print("Nachricht an Server senden: "); 
  // Serial.println(data); 
  // Daten im Offline-Speicher?
  
  while (myConnected && offlineCount > offlineSend){
    File fin = LittleFS.open("/data/01", "r");
    if (fin) {
      fin.seek((offlineSend-1)*51, SeekSet);
      if(fin.available()){
        // '\n' is not included in the returned string, but the last char '\r' is
        String line=fin.readStringUntil('\n');
        Serial.print("SE-Line: ");
        Serial.println(line);
          client.println(line); 
        delay(100); 
        /*Echo vom Server lesen und verwerfen, da alte Daten*/ 
        line = client.readStringUntil('\n'); 
        if (line.length() == 0){
          myConnected = false;
        } else {
          offlineSend++;
          File fout = LittleFS.open("/data/offline", "a");
          if (fout) {
            fout.write(offlineSend);
            int offlineData = fout.size();
            fout.close();
            Serial.print("SE-Size: ");
            Serial.println(offlineData);
          } else {
            Serial.println("file offline open failed");
          }
        }
      }
      fin.close();
    } else {
      Serial.println("file offline open failed");
    }
  }
  client.println(data); 
  delay(100); 
 
  /*Echo vom Server lesen und ausgeben*/ 
  String line = client.readStringUntil('\n'); 
  if (line.length() == 0){
    myConnected = false;
  } else {
    line.toCharArray(dataReturn, 21);
    //Serial.print("Online:");
    //Serial.print(line); 
    //Serial.println(); 
    //Serial.print("Online:");
    //Serial.print(data); 
    //Serial.println(); 
    
    displayText(2, dataReturn, 4);
  }
  /*Verbindung zum Server schliessen*/ 
  //Serial.println("Verbindung schliessen"); 
  client.flush(); 
  client.stop(); 
  //Serial.println("Verbindung geschlossen");  
  return myConnected;
}

void sendAndReplay(long id) {
    /*
  1 dppz.terminal.id    99
  2 dppz.satz.nr        0000
  3 dppz.satz.kennung   X
  4 dppz.satz.art       F0 (Automatik), sonst KO, GE, DG, SO
  5 dppz.karten.nr      50 (Bielemeier)
*/
    //snprintf(data, 80, "%s %s %10d 4d%2d%2d%2d%2d%2d       ", message[3][messageWIFI], terminal, id, year(), month(), day(),hour(), minute(), second()) ;
    //R_11J22223__44_________5555555566666666777777____
    //snprintf(data, 80, "R%3sJ%4d%c__%2s_________%08d%04d%02d%02d%02d%02d%02d____", message[3][messageWIFI], terminalId, satzNummer, satzKennung, satzArt, id, year(), month(), day(),hour(), minute(), second()) ;
    snprintf(data, 80, "R%2s1J2222%c__%2s_________%08d%04d%02d%02d%02d%02d%02d____", terminalId, satzKennung, satzArt, id, year(), month(), day(),hour(), minute(), second());

    //sendToServer();
    if (sendToServer()) {
      displayChar(messageONL, 3, 'O');
    } else {
      displayChar(messageONL, 3, '-');
      offlineCount++;
      //save to File and send later
      File f = LittleFS.open("/data/01", "a");
      if (f) {
        f.println(data);
        int offlineData = f.size();
        f.close();
        Serial.print("01-Size: ");
        Serial.print(offlineData);
        Serial.print(": ");
        Serial.println(data);
      } else {
        Serial.println("file 01 open failed");
      }
      /////////////////////////////////////////dataWrite();
      //Serial.print("Offline:");
      //Serial.println(data);
      snprintf(data, 21, "gelesen: %d         ", id) ;
      displayText(2, data, 4);
      snprintf(data, message3+2, "Offline: %d          ", offlineCount) ;
      displayText(3, data);
    }
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
      if (address == IO_I2C_ADDRESS)  IOok  = 1;
      if (address == RTC_I2C_ADDRESS) RTCok = 1;
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
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
#endif

void Zeit_Einstellen()
{
  if (NTPok) {
    if(abs(NTPTime - now()) > 5) {
      //LogSchreibenNow("falsche Zeit");
      //Serial.println( Temp );
      //setTime(NTPTime);
      //LogSchreiben("NTP: Zeit gesetzt");
      //Serial.println( Temp );
    }
  } else {
    if (WifiConnected > 0) {
      NTPTime = GetNTP();
      setTime(NTPTime);
      NTPok = (NTPTime > 0);
    } else {
      NTPTime = 0;
      NTPok = false;
    }
  }
  if (NTPok) {
    displayChar(messageNTP, 3, 'T');
  } else {
    displayChar(messageNTP, 3, '-');
  }

# ifdef IIC_RTC
    if (RTCok) {
      RTCTime = RTC.now().unixtime();
      if (NTPok) {
        if(abs(RTCTime - NTPTime) > 5) {
          Temp = PrintDate(RTCTime) + "   " + PrintTime(RTCTime) + "   falsche RTC-Zeit";
          //LogSchreiben(Temp);
          //Serial.println( Temp );
          RTCSync = NTPTime;
          RTC.adjust(DateTime(year(NTPTime), month(NTPTime), day(NTPTime), hour(NTPTime), minute(NTPTime), second(NTPTime)));
        }
      } else {
        if(abs(RTCTime - now()) > 5) {
          //LogSchreibenNow("falsche Zeit");
          setTime(RTCTime);
          //LogSchreiben("RTC: Zeit gesetzt");
        }
      }
      displayChar(messageRTC, 3, 'R');
    } else {
      displayChar(messageRTC, 3, '-');
    }
# endif
  //displayText(3, message[3]);
}

// Function to connect WiFi
void connectWifi() {
  if (WiFi.status() != WL_CONNECTED && WifiNext < myTime)
  {
    WifiConnected = 0;
    WifiCounter++;
    if (WifiCounter == 1){
      //WiFi.sta.autoconnect(0);
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);                                            // Disable AP mode
      WiFi.begin(ssid, passwort);
      displayText(1, "Connecting to:      ");
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
    }
  }
}
#ifdef IIC_KEYPAD
  #define keymapRows 4
  #define keymapCols 4
  #define keymapClick true
  boolean keymapPause = true;
  uint8_t pcf8574Last[keymapCols]= {0};
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
    if (pcf8574Active){
      test = pcf8574.read8();
      //if (test != 0x0F){
      if (test != 0xF0){
        //Serial.print("keypad1 ");
        //Serial.println(test, BIN);
        for (i = 0; i < keymapCols; i++){
          pcf8574.write8(~(0x08 >> i)); // links nach rechts
          //pcf8574.write8(0xF0 | (0x08 >> i)); // links nach rechts
          delay(3);
          test = ~(pcf8574.read8());
          //Serial.print("keypad2 ");
          //Serial.println(test, BIN);
          if (keymapPause){
            pcf8574Last[i] = 0;
            keymapPause = false;
          }
          if (test != pcf8574Last[i]){
            pcf8574Last[i] = test;
            if (test > 0x0F){
              for (j = 0; j < keymapRows; j++){
                test2 = (0x80 >> j);
                //Serial.print("keypad3 ");
                //Serial.println(test2, BIN);
                keymapX = test & test2;
                if (keymapX){
                  // keyclick(keymapClick, 220, 100);
                  //serialInSet(keymap[j][i]);
                  //if (keymap[j][i] == serialKeypadEnd){
                    //serialInSet(0x0A);
                  //}
                  Serial.print("keypad  ");
                  Serial.println(keymap[j][i]);
                  switch (keymap[j][i]){
                    case '*' : 
                      myBacklight = !myBacklight;
                      lcd.setBacklight(myBacklight); 
                      break;
                    case 'A' : snprintf(satzArt, 3, "KO"); displayText(3, "Kommen         "); break;
                    case 'B' : snprintf(satzArt, 3, "GE"); displayText(3, "Gehen          "); break;
                    case 'C' : snprintf(satzArt, 3, "KR"); displayText(3, "Gehen Krank    "); break;
                    case 'D' : snprintf(satzArt, 3, "DG"); displayText(3, "Dienstgang     "); break;
                    default  : snprintf(satzArt, 3, "FO"); displayText(3, "               "); break;
                  }
                  //displayChar(0,3, keymap[j][i]);
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
  Serial.println("configRead");
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
                                          // check 67
  LeseEepromCheck();
  EEPROM.end();
  //if (version[0] != versionNeu[0] || !LeseEepromCheck())
  { // Lese-Fehler, alles initialisieren außer ssid / passwort, 
    // da sonst der Zugang zum eventuell laufenden System zerstört wird 
    // ssid[0] = 0;
    // passwort[0] = 0;
    //UpdateServer[0] = 0;
    //strcpy(timeserver, "time.nist.gov");
    //strcpy(device_name, "Zeit");
  }
}
void configWrite() {
  z = 0;
  EEPROM.begin(512);
  SchreibeEeprom(versionNeu,2 );
  version[0] = versionNeu[0];
  SchreibeEeprom(ssid, sizeof(ssid));
  SchreibeEeprom(passwort, sizeof(passwort));
  SchreibeEeprom(UpdateServer, LOGINLAENGE);
  SchreibeEeprom(timeserver, LOGINLAENGE);
  SchreibeEeprom(device_name, LOGINLAENGE);
  SchreibeEeprom(terminalId, 4);
  SchreibeEeprom(serverHost, LOGINLAENGE);
  SchreibeEeprom(serverPort);
  SchreibeEepromCheck();
  EEPROM.commit();
  EEPROM.end();
}

void configPrint() {
  Serial.println(version);
  Serial.println(ssid);
  Serial.println(passwort);
  Serial.println(UpdateServer);
  Serial.println(timeserver);
  Serial.println(device_name);
  Serial.println(terminalId);
  Serial.println(serverHost);
  Serial.println(serverPort);
  //LeseEepromCheck();
}

void Serial_Task() {
  while (Serial.available() > 0)  
  { // Eingabe im Seriellen Monitor lesen
    char Zeichen = Serial.read();    
    byte pos = 0;
    int ziffer = 0;
    if (Zeichen == '\n') 
    { // Enter/Senden gedrückt
      eingabe[eingabePos] = 0;
      Serial.println(eingabe);
      switch (eingabe[0]){
        case 'a' : for (pos=0; pos<=eingabePos; pos++) ssid[pos] = eingabe[pos+1]; break;
        case 'b' : for (pos=0; pos<=eingabePos; pos++) passwort[pos] = eingabe[pos+1]; break;
        case 'c' : for (pos=0; pos<=eingabePos; pos++) UpdateServer[pos] = eingabe[pos+1]; break;
        case 'd' : for (pos=0; pos<=eingabePos; pos++) timeserver[pos] = eingabe[pos+1]; break;
        case 'e' : for (pos=0; pos<=eingabePos; pos++) device_name[pos] = eingabe[pos+1]; break;
        case 'f' : for (pos=0; pos<=eingabePos; pos++) terminalId[pos] = eingabe[pos+1]; break;
        case 'g' : for (pos=0; pos<=eingabePos; pos++) serverHost[pos] = eingabe[pos+1]; break;
        case 'h' : serverPort = 0; 
                   for (pos=1; pos<eingabePos; pos++) {
                     ziffer = eingabe[pos];
                     ziffer = ziffer - 48;
                     serverPort = serverPort * 10 + ziffer; 
                   }
                   break;
        
        case 'p' : configPrint(); break;
        case 'w' : configWrite(); break;
      }
      eingabePos = 0; 
    } else {
      eingabe[eingabePos++] = Zeichen;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  configRead();

  // SPI-Bus initialisieren
  SPI.begin();
  // MFRC522 initialisieren
  mfrc522.PCD_Init();

# ifdef IICTEST
    Wire.begin(PIN_SDA, PIN_SCK);
    testIIC();
    #ifdef IIC_KEYPAD
      if (IOok) {
        //mcp.begin(0);
      }
    #endif
    #ifdef IIC_DISPLAY
      if (DISPLAYok) {
        //oledSplash();
        int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));

        // Switch on the backlight
        //pinMode ( BACKLIGHT_PIN, OUTPUT );
        //digitalWrite ( BACKLIGHT_PIN, HIGH );
        lcd.begin(20,4);               // initialize the lcd 

        for ( int i = 0; i < charBitmapSize; i++ )
        {
          lcd.createChar ( i, (uint8_t *)charBitmap[i] );
        }

        lcd.home ();                   // go home
        displayText(1, "Display ok");
        
        /*for ( int i = 0; i < charBitmapSize; i++ )
        {
          lcd.createChar ( i, (uint8_t *)charBitmap[i] );
        }
      
        lcd.home ();                   // go home
        lcd.print("Hello, ARDUINO ");  
        lcd.setCursor ( 0, 1 );        // go to the next line
        lcd.print (" FORUM - fm   ");
        */
      }
    #endif
# endif
    LittleFS.begin();
    //LittleFS.remove("/data/offline");
    //LittleFS.remove("/data/01");
    File fin = LittleFS.open("/data/01", "r");
    if (fin) {
      offlineCount = fin.size()/51;
      fin.close();
    }
    File fout = LittleFS.open("/data/offline", "r");
    if (fout) {
      offlineSend = fout.size();
      fout.close();
    }
   
    
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.println("totalBytes " + String(fs_info.totalBytes)+ ", used " + String(fs_info.usedBytes));
  // Details vom MFRC522 RFID READER / WRITER ausgeben
  //Kurze Pause nach dem Initialisieren   
  //delay(10);
  //mfrc522.PCD_DumpVersionToSerial();  
}

void loop() {
  unsigned long myNow = now();

  if (chipID > 0) {
    displayText(1, "Karte bitte ...     ") ;
    displayText(2, "                    ");
    //CardID resetten
    chipID = 0;
    snprintf(satzArt, 3, "FO");
  }
  if (myTime != myNow) {
    myTime = myNow;
    myTimeFromTics = millis() / 1000;
    //WiFi check
    connectWifi();          // Try to connect WiFi aSync
    if (WifiConnected > 0 ){
      Zeit_Einstellen();
    }
    snprintf(message[0], 21, "%02d.%02d.%04d %02d:%02d:%02d ", day(), month(), year(),hour(), minute(), second()) ;
    displayText();
  }
  // Sobald ein Chip aufgelegt wird startet diese Abfrage
  if (mfrc522.PICC_IsNewCardPresent()){
    mfrc522.PICC_ReadCardSerial();

    // Hier wird die ID des Chips in die Variable chipID geladen
    for (byte i = 0; i < mfrc522.uid.size; i++){
      chipID=((chipID+mfrc522.uid.uidByte[i])*10);
    }

    //... und anschließend ausgegeben
    if (chipID > 0) {
      sendAndReplay(chipID);
    }
    
    // Danach 1 Sekunde pausieren, um mehrfaches lesen /ausführen zu verhindern
    delay(1000);
      
  }
  keypadloop();
  Serial_Task();
}