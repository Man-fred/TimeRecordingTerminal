#include "myFS.h"
#include "defines.h"
#include "log.h"

#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
extern ESP8266WebServer httpserver;
extern char UpdateServer[LOGINLAENGE];
extern String mVersionNr;
extern String mVersionVariante;
extern String mVersionBoard;
extern  const char* www_username;
extern  const char* www_password;


File fsUploadFile;

String getContentType(String filename) {
  if (httpserver.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".manifest")) return "text/cache-manifest";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
    if (LittleFS.exists(pathWithGz))
      path += ".gz";
    File file = LittleFS.open(path, "r");
    //size_t sent = httpserver.streamFile(file, contentType);
    httpserver.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFile() {
  if (!httpserver.authenticate(www_username, www_password)) {
      return httpserver.requestAuthentication();
  }
  if (!handleFileRead(httpserver.uri())){
	  handleNotFound();
  }
  /*if (is_authentified()) {
    String test =  (httpserver.uri() == "/") ? "/index.html" : httpserver.uri();
    return LittleFS.exists(test) ? ({File f = LittleFS.open(test, "r"); httpserver.streamFile(f, mime::getContentType(test)); f.close(); true;}) : httpserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>");
  } else {

  }*/
}


void handleFileDelete() {
  if (is_authentified()) {
    if (httpserver.args() == 0) return httpserver.send(500, "text/plain", "BAD ARGS");
    String path = httpserver.arg(0);
    DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
    if (path == "/")
      return httpserver.send(500, "text/plain", "BAD PATH");
    if (!LittleFS.exists(path))
      return httpserver.send(404, "text/plain", "FileNotFound");
    LittleFS.remove(path);
    httpserver.send(200, "text/plain", "");
    path = String();
  }
}

void handleFileCreate() {
  if (is_authentified()) {
    if (httpserver.args() == 0)
      return httpserver.send(500, "text/plain", "BAD ARGS");
    String path = httpserver.arg(0);
    DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
    if (path == "/")
      return httpserver.send(500, "text/plain", "BAD PATH");
    if (LittleFS.exists(path))
      return httpserver.send(500, "text/plain", "FILE EXISTS");
    File file = LittleFS.open(path, "w");
    if (file)
      file.close();
    else
      return httpserver.send(500, "text/plain", "CREATE FAILED");
    httpserver.send(200, "text/plain", "");
    path = String();
  }
}

String handleDirList(String path, int level){
	Dir dir = LittleFS.openDir(path);
    String output = "";
    
	while (dir.next()) {
	  bool isDir = dir.isDirectory();
	  output += "{\"type\":\"";
	  output += (isDir) ? "dir" : "file";
	  output += "\",\"name\":\"";
	  if (isDir)
	      output += path + dir.fileName() + "/";//.substring(1);
	  else
		  output += path + dir.fileName();
	  output += "\",\"size\":\"";
	  File entry = dir.openFile("r");
	  output += String(entry.size());
	  entry.close();
	  output += "\"},";
	  if (isDir && level > 0){
		output += handleDirList(path + dir.fileName() + "/",level-1);
	  }
	}
	return output;
}

void handleFileList() {
  String output = "[";
  if (is_authentified()) {
    if (!httpserver.hasArg("dir")) {
      httpserver.send(500, "text/plain", "BAD ARGS");
      return;
    }
	
    String path = httpserver.arg("dir");
	int level = httpserver.arg("level").toInt();
    DBG_OUTPUT_PORT.println("handleFileList: " + path);
	output += handleDirList(path,level);
  }
  output += "{}]";
  httpserver.send(200, "text/json", output);
}


void handleNotAllowed() {
  String message = "<pre>File Not Allowed\n\n";

  httpserver.send ( 404, "text/html", message );
}

/*
void update_Version() {
  if (is_authentified()) {
    httpserver.sendHeader("Location", "/index.htm");
    httpserver.send(303, "text/html", ""); // Antwort an Internet Browser
    t_httpUpdate_return ret = ESPhttpUpdate.update(UpdateServer, 80, "/esp8266/ota.php", (mVersionNr + mVersionVariante + mVersionBoard).c_str());
  
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        LogSchreibenNow("[update] " + mVersionNr + mVersionVariante + mVersionBoard + " Update failed.");
        LogSchreibenNow(/ *ESPhttpUpdate.getLastError() +" "+* / ESPhttpUpdate.getLastErrorString().c_str());
        break;
      case HTTP_UPDATE_NO_UPDATES:
        DBG_OUTPUT_PORT.println("[update] " + mVersionNr + mVersionVariante + mVersionBoard + " No update.");
        break;
      case HTTP_UPDATE_OK:
        LogSchreibenNow("[update] " + mVersionNr + mVersionVariante + mVersionBoard + " Update ok."); // may not called, we reboot the ESP
        break;
      default:
        LogSchreibenNow("[update] " + mVersionNr + mVersionVariante + mVersionBoard + " unknown error");
        LogSchreibenNow(/ *ESPhttpUpdate.getLastError() +" "+* / ESPhttpUpdate.getLastErrorString().c_str());
        break;
    }
  }
}
*/


