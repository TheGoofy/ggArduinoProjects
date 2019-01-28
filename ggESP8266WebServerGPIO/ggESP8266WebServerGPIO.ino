#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>      // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h>
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager

#include "ggOutputPins.h"
#include "ggHtmlData.h"
#include "ggClients.h"
#include "ggConfig.h"
#include "ggStringStream.h"


String mHostName = "ESP8266-" + ggConfig::GetChipId();


// Create web server on port 80
ESP8266WebServer mServer(80);

// Create web sockets server on port 81
WebSocketsServer mWebSockets(81);

// interface for clients communication via websockets
ggClients mClients(mWebSockets);


void ToggleOutput(int aIndex)
{
  // get the inverted pin value from output
  bool aValue = !ggOutputPins::Get(aIndex);

  // set the inverted pin value to output
  ggOutputPins::Set(aIndex, aValue);

  // update (status) display on all connected clients
  mClients.UpdateDisplay(aIndex, aValue);
}


void WebSocketEvent(uint8_t aClientNumber,
                    WStype_t aEventType,
                    uint8_t* aPayLoad,
                    size_t aPayLoadLength)
{
  switch (aEventType) {
    case WStype_CONNECTED: {
      const String vURL((char*)aPayLoad);
      mClients.UpdateDisplay(aClientNumber);
      break;
    }
    case WStype_TEXT: {
      const String vText((char*)aPayLoad);
      if (vText == "ToggleOutput(0)") { ToggleOutput(0); return; }
      if (vText == "ToggleOutput(1)") { ToggleOutput(1); return; }
      if (vText == "ToggleOutput(2)") { ToggleOutput(2); return; }
      if (vText == "ToggleOutput(3)") { ToggleOutput(3); return; }
      break;
    }
  }  
}


void ServerOnRoot()
{
  mServer.send_P(200, "text/html", mHtmlRoot);
}


void ServerOnDebug()
{
  String vDebugString;
  ggStringStream vDebugStream(vDebugString);
  ggConfig::Print(vDebugStream);
  ggOutputPins::Print(vDebugStream);
  mServer.send(200, "text/plain", vDebugString);
}


void ServerOnReset()
{
  mServer.send(200, "text/plain", "Resetting wifi manager ...\n");
  WiFiManager mWifiManager;
  mWifiManager.setDebugOutput(false);
  mWifiManager.resetSettings();
}


void ServerOnNotFound()
{
  mServer.send(404, "text/plain", "url not found");
}


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");

  // connect to wifi
  WiFiManager mWifiManager;
  mWifiManager.setDebugOutput(true);
  mWifiManager.autoConnect(mHostName.c_str());
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // start multicast DNS
  MDNS.begin(mHostName.c_str(), WiFi.localIP());
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  Serial.printf("MDNS responder started: http://%s.local\n", mHostName.c_str());
  
  // configure and start web-server
  mServer.on("/", ServerOnRoot);
  mServer.on("/debug", ServerOnDebug);
  mServer.on("/reset", ServerOnReset);
  mServer.onNotFound(ServerOnNotFound);
  mServer.begin();
  Serial.println("Web server started");

  // configure and start web-sockets
  mWebSockets.onEvent(WebSocketEvent);
  mWebSockets.begin();
  Serial.println("Web sockets started");

  // make sure all status and debug messages are sent before communication gets
  // interrupted, in case hardware pins are needed for some different use.
  Serial.flush();

  // configure the GPIO pins
  ggOutputPins::Begin();
}


void loop()
{
  mServer.handleClient();
  mWebSockets.loop();
  MDNS.update();
}
