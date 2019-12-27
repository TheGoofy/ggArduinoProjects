#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "ggPersistT.h"
#include "ggStringConvertNumbers.h"
#include "ggTestEnum.h"

#ifndef STASSID
#define STASSID "Grofunkel"
#define STAPSK  "goofy123"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

String getContentType(String filename){
  if (filename.endsWith(".htm")) return "text/html";
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".gif")) return "image/gif";
  if (filename.endsWith(".jpg")) return "image/jpeg";
  if (filename.endsWith(".ico")) return "image/x-icon";
  if (filename.endsWith(".xml")) return "text/xml";
  if (filename.endsWith(".pdf")) return "application/x-pdf";
  if (filename.endsWith(".zip")) return "application/x-zip";
  if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}

void handleNotFound() {
  if (handleFileRead(server.uri())) return; 
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}



void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  SPIFFS.begin();
  //SPIFFS.format();

  File vFile = SPIFFS.open("/controller/info.txt", "w");
  if (vFile) {
    vFile.print("gugus");
    vFile.close();
  }

  ggPersistT<bool> vControllerEnable("/controller/enable.txt", false);
  ggPersistT<int> vControllerMode("/controller/mode.txt", 3);
  ggPersistT<float> vControllerTemperatureRef("/controller/temperature.ref.txt", 20.0f);

  Serial.printf("enable = %d\n", vControllerEnable.Get());
  Serial.printf("mode = %d\n", vControllerMode.Get());
  Serial.printf("ref = %f\n", vControllerTemperatureRef.Get());

  vControllerEnable.Set(true);
  vControllerMode.Set(3);
  vControllerTemperatureRef.Set(42.7f);

  ggPersistT<ggTestEnum::tEnum> vControllerEnum("/controller/enum.txt", ggTestEnum::eEnumA);
  Serial.printf("test enum = %s\n", ToString(vControllerEnum.Get()).c_str());
  vControllerEnum.Set(ggTestEnum::eEnumB);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
