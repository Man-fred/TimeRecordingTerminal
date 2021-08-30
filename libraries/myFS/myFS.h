#ifndef MYFS_H
#define MYFS_H

#include "Arduino.h"
#include <LittleFS.h>

extern byte is_authentified();
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
void handleNotFound();
void update_Version();
#endif

