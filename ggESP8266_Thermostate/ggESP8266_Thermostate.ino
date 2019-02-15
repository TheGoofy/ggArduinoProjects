#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>      // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager
#include <DHTesp.h>           // https://github.com/beegee-tokyo/DHTesp

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggPeriphery.h"
#include "ggController.h"

String mHostName = "ESP-SSR-" + String(ESP.getChipId(), HEX);

// runs AP, if no wifi connection
WiFiManager mWifiManager;
  
// create web server on port 80
ggWebServer mWebServer(80);

// create web sockets server on port 81
ggWebSockets mWebSockets(81);

// input and ouput configuration
ggPeriphery mPeriphery;

// controls an output based on input and reference value
ggController mController;


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");
  
  // connect to wifi
  mWifiManager.setDebugOutput(true);
  mWifiManager.autoConnect(mHostName.c_str());
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

  // when a new client is conneted, it needs a complete update
  mWebSockets.OnClientConnect([&] (int aClientID) {
    mWebSockets.UpdateKey(mPeriphery.mKey.GetPressed(), aClientID);
    mWebSockets.UpdateSensorStatus(mPeriphery.mSensor.GetStatus(), aClientID);
    mWebSockets.UpdateTemperature(mPeriphery.mSensor.GetTemperature(), aClientID);
    mWebSockets.UpdateHumidity(mPeriphery.mSensor.GetHumidity(), aClientID);
    mWebSockets.UpdateControlMode(mController.GetMode(), aClientID);
    mWebSockets.UpdateTemperatureRef(mController.GetReference(), aClientID);
    mWebSockets.UpdateOutput(mController.GetOutput(), aClientID);
  });
  
  // controller event: when output changes, the SSR needs to be switched
  mController.OnOutputChanged([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue > 0.5f);
    mWebSockets.UpdateOutput(aOutputValue);
  });

  // when button "key" is pressed we switch off the SSR (emergency)
  mPeriphery.mKey.OnChanged([&] (bool aPressed) {
    mController.SetMode(ggController::eModeOff);
    mWebSockets.UpdateControlMode(mController.GetMode());
    mWebSockets.UpdateKey(aPressed);
  });
  mPeriphery.mKey.OnReleased([&] () {
    if (mPeriphery.mKey.GetMillisDelta() > 2000) {
      mWifiManager.resetSettings();
      mController.SetReference(0.0f);
      mWebSockets.UpdateTemperatureRef(mController.GetReference());
    }
  });

  // connect sensor events
  mPeriphery.mSensor.OnStatusChanged([&] (const char* aStatus) {
    mWebSockets.UpdateSensorStatus(aStatus);
  });
  mPeriphery.mSensor.OnTemperatureChanged([&] (float aTemperature) {
    mController.SetInput(aTemperature);
    mWebSockets.UpdateTemperature(aTemperature);
  });
  mPeriphery.mSensor.OnHumidityChanged([&] (float aHumidity) {
    mWebSockets.UpdateHumidity(aHumidity);
  });

  // events from client: control mode, reference temperature
  mWebSockets.OnSetControlMode([&] (int aControlMode) {
    mController.SetMode(static_cast<ggController::tMode>(aControlMode));
    mWebSockets.UpdateControlMode(mController.GetMode());
  });
  mWebSockets.OnSetTemperatureRef([&] (float aTemperatureRef) {
    mController.SetReference(aTemperatureRef);
    mWebSockets.UpdateTemperatureRef(mController.GetReference());
  });
}

void loop()
{
  mPeriphery.Run();
  mWebServer.Run();
  mWebSockets.Run();
}

