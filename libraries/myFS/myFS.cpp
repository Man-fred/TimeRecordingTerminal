#include "myFS.h"
#include "defines.h"
#include "log.h"

#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
extern ESP8266WebServer httpserver;
extern char UpdateServer[LOGINLAENGE];
//extern String mVersionNr;
//extern String mVersionVariante;
//extern String mVersionBoard;
extern  const char* www_username;
extern  char www_password[21];


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
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz))
	path += ".gz";
  if (LittleFS.exists(path)) {
    DBG_OUTPUT_PORT.print(contentType); 
	DBG_OUTPUT_PORT.print(" handleFile Stream: "); 
    File f = LittleFS.open(path, "r");
	char buf[1024];
	int siz = f.size();
	DBG_OUTPUT_PORT.print(siz);DBG_OUTPUT_PORT.print(" ");DBG_OUTPUT_PORT.print(f.name());DBG_OUTPUT_PORT.print(" ");
	if(false){
		while(siz > 0) {
		  size_t len = std::min((int)(sizeof(buf) - 1), siz);
		  f.read((uint8_t *)buf, len);
		  httpserver.client().write((const char*)buf, len);
		  siz -= len;
		}
	} else {
		size_t sent = httpserver.streamFile(f, contentType);
		DBG_OUTPUT_PORT.print(sent); 
	}
	DBG_OUTPUT_PORT.println(path);
    f.close();
    return true;
  }
  return false;
}

void handleFile() {
  DBG_OUTPUT_PORT.print("handleFile Start: "); DBG_OUTPUT_PORT.println(httpserver.uri());
  if (!httpserver.authenticate(www_username, www_password)) {
      return httpserver.requestAuthentication();
  }
  DBG_OUTPUT_PORT.println("handleFile auth ok: ");
  String test =  (httpserver.uri() == "/") ? "/index.html" : httpserver.uri();
  if (!handleFileRead(test)){
      DBG_OUTPUT_PORT.println("handleFile not found: ");
	  handleNotFound();
  }
  /*if (is_authentified()) {
    return LittleFS.exists(test) ? ({File f = LittleFS.open(test, "r"); httpserver.streamFile(f, mime::getContentType(test)); f.close(); true;}) : httpserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>");
  }*/
}

void handleNotFound() {
  String message = "<pre>File Not Found\n\n";
  message += "URI: ";
  message += httpserver.uri();
  message += "\nMethod: ";
  message += ( httpserver.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpserver.args();
  message += "\n";

  for ( uint8_t i = 0; i < httpserver.args(); i++ ) {
    message += " " + httpserver.argName ( i ) + ": " + httpserver.arg ( i ) + "\n";
  }
  message += "</pre><br /><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>";

  httpserver.send ( 404, "text/html", message );
}

void handleUpload(){
  httpserver.send(200, "text/json", "{\"result\":\"ok\"}");
}

void handleFileUpload() {
	HTTPUpload& upload = httpserver.upload();
	if (upload.status == UPLOAD_FILE_START) {
	  String filename = upload.filename;
	  if (!filename.startsWith("/")) filename = "/" + filename;
	  DBG_OUTPUT_PORT.print("handleFileUpload Start: "); DBG_OUTPUT_PORT.println(filename);
	  fsUploadFile = LittleFS.open(filename, "w");
	  filename = String();
	} else if (upload.status == UPLOAD_FILE_WRITE) {
	  if (fsUploadFile) {
		fsUploadFile.write(upload.buf, upload.currentSize);
		DBG_OUTPUT_PORT.print("handleFileUpload Data : "); DBG_OUTPUT_PORT.println(upload.currentSize);
	  }
	} else if (upload.status == UPLOAD_FILE_END) {
	  if (fsUploadFile) {
		fsUploadFile.close();
		DBG_OUTPUT_PORT.print("handleFileUpload End  : "); DBG_OUTPUT_PORT.println(upload.totalSize);
	  }
	} else if (upload.status == UPLOAD_FILE_ABORTED) {
		DBG_OUTPUT_PORT.println("handleFileUpload Aborted");
	}
}

void handleFileDelete() {
  if (is_authentified()) {
    if (httpserver.args() == 0) return httpserver.send(500, "text/json", "{\"result\":\"BAD ARGS");
    String path = httpserver.arg(0);
    DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
    if (path == "/" || path == "/data/01")
      return httpserver.send(500, "text/json", "{\"result\":\"BAD PATH\"}");
    if (!LittleFS.exists(path))
      return httpserver.send(404, "text/json", "{\"result\":\"FileNotFound\"}");
    LittleFS.remove(path);
    httpserver.send(200, "text/json", "{\"result\":\"ok\"}");
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


