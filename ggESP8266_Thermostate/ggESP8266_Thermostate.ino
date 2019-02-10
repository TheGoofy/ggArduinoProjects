#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>      // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager
#include <DHTesp.h>           // https://github.com/beegee-tokyo/DHTesp

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggPeriphery.h"

String mHostName = "ESP8266-" + String(ESP.getChipId(), HEX);


// create web server on port 80
ggWebServer mWebServer(80);

// create web sockets server on port 81
ggWebSockets mWebSockets(81);

// input and ouput configuration
ggPeriphery mPeriphery;


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");
  
  // connect to wifi
  WiFiManager vWifiManager;
  vWifiManager.setDebugOutput(true);
  vWifiManager.autoConnect(mHostName.c_str());
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // configure and start web-server
  mWebServer.Begin();
  Serial.println("Web server started");

  // configure and start web-sockets
  mWebSockets.Begin();
  Serial.println("Web sockets started");
  
  // make sure all status and debug messages are sent before communication gets
  // interrupted, in case hardware pins are needed for some different use.
  Serial.flush();
  
  // setup connected hardware
  mPeriphery.Begin();

  // connect events ...
  mPeriphery.mSensor.OnStatusChanged([&] (const char* aStatus) {
    mWebSockets.UpdateSensorStatus(aStatus);
  });
  mPeriphery.mSensor.OnTemperatureChanged([&] (float aTemperature) {
    mWebSockets.UpdateTemperature(aTemperature);
  });
  mPeriphery.mSensor.OnHumidityChanged([&] (float aHumidity) {
    mWebSockets.UpdateHumidity(aHumidity);
  });
  mWebSockets.OnClientConnect([&] (int aClientID) {
    mWebSockets.UpdateSensorStatus(mPeriphery.mSensor.GetStatus(), aClientID);
    mWebSockets.UpdateTemperature(mPeriphery.mSensor.GetTemperature(), aClientID);
    mWebSockets.UpdateHumidity(mPeriphery.mSensor.GetHumidity(), aClientID);
  });
  mWebSockets.OnSetTemperatureRef([&] (float aTemperatureRef) {
    mWebSockets.UpdateTemperatureRef(aTemperatureRef);
  });
}

void loop()
{
  mPeriphery.Run();
  mWebServer.Run();
  mWebSockets.Run();
}

