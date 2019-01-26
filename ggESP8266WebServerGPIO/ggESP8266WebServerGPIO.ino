#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>

#include "ggOutputPins.h"
#include "ggHtmlData.h"
#include "ggClients.h"


const char* mWiFiSSID = "........";
const char* mWiFiPassword = "........";
const char* mMdnsHostName = "ESP8266-GPIO";


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
  WiFi.mode(WIFI_STA);
  WiFi.begin(mWiFiSSID, mWiFiPassword);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // start multicast DNS
  MDNS.begin(mMdnsHostName, WiFi.localIP());
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  Serial.println("MDNS responder started");
  
  // configure and start web-server
  mServer.on("/", ServerOnRoot);
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
}

