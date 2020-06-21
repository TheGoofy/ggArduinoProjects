#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("startup");

  SPIFFS.begin();

  WiFiManager wm;
  wm.setDebugOutput(true);
  wm.setConfigPortalTimeout(60); // 1 minute
  wm.autoConnect("goofy");
  Serial.printf("Connected to: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  
  // start mdns
  MDNS.begin("goofy");
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  // over the air update
  ArduinoOTA.setHostname("goofy");
  ArduinoOTA.begin();
}

void loop()
{
  static unsigned long vMillisLast = 0;
  unsigned long vMillis = millis();
  unsigned long vMillisDelta = vMillis - vMillisLast;
  if (vMillisDelta > 1000) {
    vMillisLast = vMillis;
    static int vCount = 5;
    vCount--;
    Serial.printf("count = %d\n", vCount);
    if (vCount == 0) {
      Serial.println("restart");
      Serial.flush();
      ESP.restart();
    }
  }
  
  MDNS.update();
  ArduinoOTA.handle();
  yield();
}
