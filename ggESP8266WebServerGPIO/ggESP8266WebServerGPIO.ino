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

// convenienc interface for clients communication via websockets
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
    }
    case WStype_TEXT: {
      const String vText((char*)aPayLoad);
      if (vText == "ToggleOutput(0)") { ToggleOutput(0); return; }
      if (vText == "ToggleOutput(1)") { ToggleOutput(1); return; }
      if (vText == "ToggleOutput(2)") { ToggleOutput(2); return; }
      if (vText == "ToggleOutput(3)") { ToggleOutput(3); return; }
    }
  }  
}


void ServerOnRoot()
{
  mServer.send_P(200, "text/html", mHtmlRoot);
}


void ServerOnNotFound()
{
  mServer.send(404, "text/plain", "fail");
}


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  
  // connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(mWiFiSSID, mWiFiPassword);
  Serial.println("");

  // wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Multicast DNS
  if (MDNS.begin(mMdnsHostName, WiFi.localIP())) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
  }
  
  // install various handlers
  mServer.on("/", ServerOnRoot);
  mServer.onNotFound(ServerOnNotFound);
  
  // start the server
  mServer.begin();
  Serial.println("HTTP server started");

  // start websockets
  mWebSockets.begin();
  mWebSockets.onEvent(WebSocketEvent);
  Serial.println("Web sockets started");
  Serial.flush();

  // configure GPIO pins.
  // serial communication gets interrupted, if those pins are interfering
  ggOutputPins::Begin();
}


void loop()
{
  mServer.handleClient();
  mWebSockets.loop();
}

