#ifndef MYFS_H
#define MYFS_H

#include "Arduino.h"
#include <LittleFS.h>

#ifdef USE_HTTP
  #include <ESP8266WebServer.h>
#else
  #include <ESP8266WebServerSecure.h>
#endif

extern byte is_authentified();

String getContentType(String filename);
bool handleFileRead(String path);
void handleFile();
void handleNotFound();
void handleUpload();
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
String handleDirList(String path, int level);
void handleFileList();
void handleNotAllowed();
//void update_Version();

#endif

