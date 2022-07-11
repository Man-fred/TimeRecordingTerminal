#ifndef MYFS_H
#define MYFS_H

#include "Arduino.h"
#include <LittleFS.h>

extern byte is_authentified();
String getContentType(String filename);
void handleFile();
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
void handleNotFound();
void handleNotAllowed();
//void update_Version();

String handleDirList(String path, int level);
#endif

