#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include "ggWiFiManager.h"

const String mHostName = "WiFiMgr-ESP-" + String(ESP.getChipId(), HEX);
ggWiFiManager mWiFiManager(mHostName);
ESP8266WebServer mWebServer(80);
const byte DNS_PORT = 53;
DNSServer mDnsServer;


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");

  WiFi.mode(WIFI_STA);
  // WiFi.begin("Grofunkel", "goofy123");
  mWiFiManager.Begin();

  mWebServer.on("/", [] () {
    mWebServer.send(200, "text/html", "<h1>goofy is there</h1>");
  });

  mWebServer.on("/goofy", [] () {
    mWebServer.send(200, "text/html", "<h1>goofy is here</h1>");
  });

  mWebServer.on("/ap_start", [] () {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(mHostName.c_str(), "goofy123");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    mDnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    mDnsServer.start(DNS_PORT, "*", myIP);
    mWebServer.send(200, "text/html", "<h1>ap_start</h1>");
  });

  mWebServer.on("/ap_stop", [] () {
    mWebServer.send(200, "text/html", "<h1>ap_stop</h1>");
    WiFi.mode(WIFI_STA);
  });

  mWebServer.on("/scan", [] () {
    int vNumNetworks = WiFi.scanNetworks();
    for (int vIndex = 0; vIndex < vNumNetworks; vIndex++) {
      String vSSID = WiFi.SSID(vIndex);
      Serial.printf("%d %s %d\n", vIndex, vSSID.c_str(), WiFi.RSSI(vIndex));
    }
    mWebServer.send(200, "text/html", "<h1>scan</h1>");
  });

  mWebServer.on("/reset", [] () {
    mWebServer.send(200, "text/html", "<h1>reset</h1>");
    WiFi.disconnect(true);
  });

  mWebServer.onNotFound([]() {
    bool vIsValidIP = IPAddress::isValid(mWebServer.hostHeader());
    if (!vIsValidIP) {
      Serial.printf("\"%s\" - is not a valid IP\n", mWebServer.hostHeader().c_str());
      String vHeader = String("http://") + mWebServer.client().localIP().toString() + String("/scan");
      Serial.printf("redirecting to \"%s\"\n", vHeader.c_str());
      mWebServer.sendHeader("Location", vHeader, true);
      mWebServer.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
      mWebServer.client().stop(); // Stop is needed because we sent no content length
      return;
    }
    mWebServer.send(200, "text/html", "<h1>goofy is not here</h1>");
  });

  mWiFiManager.OnConnect([&] () {
    Serial.println("Wifi connected");
    mWiFiManager.Print(Serial);
  });

  mWiFiManager.OnDisconnect([&] () {
    Serial.println("Wifi disconnected");
  });

  mWebServer.begin();

}


void loop()
{
  mWiFiManager.Run();
  mDnsServer.processNextRequest();
  mWebServer.handleClient();
  yield();
}
