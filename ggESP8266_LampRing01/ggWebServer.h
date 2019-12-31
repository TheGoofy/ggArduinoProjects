#pragma once

#include <FS.h>

#include "ggWebServerHtmlData.h"
#include "ggDebug.h"

class ggWebServer {

public:

  ggWebServer(int aPort)
  : mServer(aPort) {
  }

  void Begin() {
    mServer.on("/", [&] () { OnController(); });
    mServer.on("/controller", [&] () { OnController(); });
    mServer.onNotFound([&] () { OnNotFound(); });
    mServer.begin();
    SPIFFS.begin();
  }

  void Run() {
    mServer.handleClient();
  }

private:

  static String GetContentType(const String& aFileName) {
    if (aFileName.endsWith(".htm")) return "text/html";
    if (aFileName.endsWith(".html")) return "text/html";
    if (aFileName.endsWith(".css")) return "text/css";
    if (aFileName.endsWith(".js")) return "application/javascript";
    if (aFileName.endsWith(".png")) return "image/png";
    if (aFileName.endsWith(".gif")) return "image/gif";
    if (aFileName.endsWith(".jpg")) return "image/jpeg";
    if (aFileName.endsWith(".ico")) return "image/x-icon";
    if (aFileName.endsWith(".xml")) return "text/xml";
    if (aFileName.endsWith(".pdf")) return "application/x-pdf";
    if (aFileName.endsWith(".zip")) return "application/x-zip";
    if (aFileName.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
  }

  bool HandleFile(const String& aFileName) {
    GG_DEBUG();
    vDebug.PrintF("aFileName = %s\n", aFileName.c_str());
    if (SPIFFS.exists(aFileName)) {
      vDebug.PrintF("file exists\n");
      File vFile = SPIFFS.open(aFileName, "r");
      if (vFile) {
        vDebug.PrintF("file opened\n");
        const String vContentType = GetContentType(aFileName);
        size_t sent = mServer.streamFile(vFile, vContentType);
        vFile.close();
        return true;
      }
    }
    return false;
  }

  void OnNotFound() {
    if (HandleFile(mServer.uri())) return;
    mServer.send(404, "text/plain", "url not found");
  }

  void OnController() {
    GG_DEBUG();
    vDebug.PrintF("local IP = %s\n", mServer.client().localIP().toString().c_str());
    vDebug.PrintF("remote IP = %s\n", mServer.client().remoteIP().toString().c_str());
    mServer.send_P(200, "text/html", mWebServerHtmlRoot);
  }

  ESP8266WebServer mServer;

};
