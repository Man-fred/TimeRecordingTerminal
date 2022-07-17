#ifndef MYFS_H
#define MYFS_H

#include "Arduino.h"
#include <LittleFS.h>

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

