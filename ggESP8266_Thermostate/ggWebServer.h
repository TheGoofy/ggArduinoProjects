#pragma once

#include "ggWebServerHtmlData.h"

class ggWebServer {

public:

  ggWebServer(int aPort)
  : mServer(aPort) {
  }

  void Begin() {
    mServer.on("/", [&] () { OnRoot(); });
    mServer.onNotFound([&] () { OnNotFound(); });
    mServer.begin();
  }

  void Run() {
    mServer.handleClient();
  }

private:

  void OnRoot() {
    mServer.send_P(200, "text/html", mWebServerHtmlRoot);
  }

  void OnNotFound() {
    mServer.send(404, "text/plain", "url not found");
  }

  ESP8266WebServer mServer;

};

