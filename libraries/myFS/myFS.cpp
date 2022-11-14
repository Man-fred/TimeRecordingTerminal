#include "myFS.h"
#include "defines.h"
#include "log.h"

#include <ESP8266httpUpdate.h>
#ifdef USE_HTTP
  ESP8266WebServer httpserver(80);
#else
static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDHjCCAgagAwIBAgIIFZifFje1WHcwDQYJKoZIhvcNAQELBQAwFzEVMBMGA1UE
AxMMQmllbGVtZWllckNBMB4XDTIyMDcyMDE5MzAwMFoXDTIzMDcyMDE5MzAwMFow
GTEXMBUGA1UEAxMOdHQ5My5mcml0ei5ib3gwggEiMA0GCSqGSIb3DQEBAQUAA4IB
DwAwggEKAoIBAQD1SgcG2fig0peNGAg5D+svEr+IjRrxDmYPGF+7Br3/8WR+1Igg
VjxSEaTZg0mmd8o7JhapurC4mEhknawQhanbfTU6dq+4YGuko6AEkmWOAfaX/hm5
Q8uZtdCg8fdARSl3TzAi0/qO+jcfT6Qr8NdwPOPMBzRt26QYCLfsP7yQl925EZ/M
hGAFOhnTG2Hu7+OOoV95GOVJoHiRa/cc9ZKugM2KzLf1B4TczkvkHJCWmNDhlr5k
zbKWN8Qm2xbSA5AnQUvAmQMPorZ6H751likBF5beqLT4eZ48Mvjtbf0PmezjA2EG
h9NiEMd9fPP5FtxczmN9pJxEdbs45kTvo4QPAgMBAAGjbDBqMAwGA1UdEwEB/wQC
MAAwHQYDVR0OBBYEFOZU6uV8Hobn/zw7csHbqgoeHi7lMAsGA1UdDwQEAwID6DAT
BgNVHSUEDDAKBggrBgEFBQcDATAZBgNVHREEEjAQgg50dDkzLmZyaXR6LmJveDAN
BgkqhkiG9w0BAQsFAAOCAQEAkGxYZtPSMvhAc0TRibURuiFOhv661KqUihYCXJA8
L9fG0gNdnaXSxU/1v5jpJ11RH49Ckiva35FJqYmwde0NydhTcd1nCiGCwte0jo5p
4wXnmkgMws3xtD0/DIZ++Pp2E+gz86g89W3snWHYfVHyAOHOEZXgIgpwlRmL7o9I
F7xaCM7u5PxV/+oaTP634rVrqR7II72SWe/dddwyNtgt/9wuap5FPJU3mqTaOkHm
OUmWSheeT410TsVhUvQNJTVvacI+kUVeCZoRTUITu+QWpLs1qg/HBZIo/9kGwJw+
OC0iaWCQJd4oNpVJ9F+OwbUGrNepjqI/UOriXXKT/3l42g==
-----END CERTIFICATE-----
)EOF";

static const char serverPrivKey[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA9UoHBtn4oNKXjRgIOQ/rLxK/iI0a8Q5mDxhfuwa9//FkftSI
IFY8UhGk2YNJpnfKOyYWqbqwuJhIZJ2sEIWp2301OnavuGBrpKOgBJJljgH2l/4Z
uUPLmbXQoPH3QEUpd08wItP6jvo3H0+kK/DXcDzjzAc0bdukGAi37D+8kJfduRGf
zIRgBToZ0xth7u/jjqFfeRjlSaB4kWv3HPWSroDNisy39QeE3M5L5ByQlpjQ4Za+
ZM2yljfEJtsW0gOQJ0FLwJkDD6K2eh++dZYpAReW3qi0+HmePDL47W39D5ns4wNh
BofTYhDHfXzz+RbcXM5jfaScRHW7OOZE76OEDwIDAQABAoIBAQDKov5NFbNFQNR8
djcM1O7Is6dRaqiwLeH4ZH1pZ3d9QnFwKanPdQ5eCj9yhfhJMrr5xEyCqT0nMn7T
yEIGYDXjontfsf8WxWkH2TjvrfWBrHOIOx4LJEvFzyLsYxiMmtZXvy6YByD+Dw2M
q2GH/24rRdI2klkozIOyazluTXU8yOsSGxHr/aOa9/sZISgLmaGOOuKI/3Zqjdhr
eHeSqoQFt3xXa8jw01YubQUDw/4cv9rk2ytTdAoQUimiKtgtjsggpP1LTq4xcuqN
d4jWhTcnorWpbD2cVLxrEbnSR3VuBCJEZv5axg5ZPxLEnlcId8vMtvTRb5nzzszn
geYUWDPhAoGBAPyKVNqqwQl44oIeiuRM2FYenMt4voVaz3ExJX2JysrG0jtCPv+Y
84R6Cv3nfITz3EZDWp5sW3OwoGr77lF7Tv9tD6BptEmgBeuca3SHIdhG2MR+tLyx
/tkIAarxQcTGsZaSqra3gXOJCMz9h2P5dxpdU+0yeMmOEnAqgQ8qtNBfAoGBAPim
RAtnrd0WSlCgqVGYFCvDh1kD5QTNbZc+1PcBHbVV45EmJ2fLXnlDeplIZJdYxmzu
DMOxZBYgfeLY9exje00eZJNSj/csjJQqiRftrbvYY7m5njX1kM5K8x4HlynQTDkg
rtKO0YZJxxmjRTbFGMegh1SLlFLRIMtehNhOgipRAoGBAPnEEpJGCS9GGLfaX0HW
YqwiEK8Il12q57mqgsq7ag7NPwWOymHesxHV5mMh/Dw+NyBi4xAGWRh9mtrUmeqK
iyICik773Gxo0RIqnPgd4jJWN3N3YWeynzulOIkJnSNx5BforOCTc3uCD2s2YB5X
jx1LKoNQxLeLRN8cmpIWicf/AoGBANjRSsZTKwV9WWIDJoHyxav/vPb+8WYFp8lZ
zaRxQbGM6nn4NiZI7OF62N3uhWB/1c7IqTK/bVHqFTuJCrCNcsgld3gLZ2QWYaMV
kCPgaj1BjHw4AmB0+EcajfKilcqtSroJ6MfMJ6IclVOizkjbByeTsE4lxDmPCDSt
/9MKanBxAoGAY9xo741Pn9WUxDyRplww606ccdNf/ksHWNc/Y2B5SPwxxSnIq8nO
j01SmsCUYVFAgZVOTiiycakjYLzxlc6p8BxSVqy6LlJqn95N8OXoQ+bkwUux/ekg
gz5JWYhbD6c38khSzJb0pNXCo3EuYAVa36kDM96k1BtWuhRS10Q1VXk=
-----END RSA PRIVATE KEY-----
)EOF";
// Attach the server private cert/key combo
BearSSL::X509List *server_cert = new BearSSL::X509List(serverCert);
BearSSL::PrivateKey *server_key = new BearSSL::PrivateKey(serverPrivKey);

BearSSL::ESP8266WebServerSecure httpserver(443);
//BearSSL::ServerSessions serverCache(5);
httpserver.setRSACert(server_cert, server_key);


#endif

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
  if (path.endsWith("/")) path += "index.htm";
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
		  #ifdef USE_HTTP
		    httpserver.client().write((const char*)buf, len);
		  #else
		    httpserver.client().write((const uint8_t*)buf, len);
		  #endif
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
  if (!handleFileRead(httpserver.uri())){
      DBG_OUTPUT_PORT.println("handleFile not found: ");
	  handleNotFound();
  }
  /*if (is_authentified()) {
    String test =  (httpserver.uri() == "/") ? "/index.html" : httpserver.uri();
    return LittleFS.exists(test) ? ({File f = LittleFS.open(test, "r"); httpserver.streamFile(f, mime::getContentType(test)); f.close(); true;}) : httpserver.send(200, "text/html", "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>");
  } else {

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


