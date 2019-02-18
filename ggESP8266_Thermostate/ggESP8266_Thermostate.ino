#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager
#include <DHTesp.h>           // https://github.com/beegee-tokyo/DHTesp

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggController.h"

const String mHostName = "ESP-SSR-" + String(ESP.getChipId(), HEX);

// runs AP, if no wifi connection
WiFiManager mWifiManager;
ggWiFiConnection mWiFiConnection;
  
// create web server on port 80
ggWebServer mWebServer(80);

// create web sockets server on port 81
ggWebSockets mWebSockets(81);

// input and ouput configuration
ggPeriphery mPeriphery;

// controls an output based on input and reference value
ggController mTemperatureController;


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
  mWiFiConnection.Begin();

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
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode(), aClientID);
    mWebSockets.UpdateTemperatureRef(mTemperatureController.GetReference(), aClientID);
    mWebSockets.UpdateOutput(mTemperatureController.GetOutput(), aClientID);
  });
  
  // controller event: when output changes, the SSR needs to be switched
  mTemperatureController.OnOutputChanged([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
  });

  // when button "key" is pressed we switch off the SSR (emergency)
  mPeriphery.mKey.OnChanged([&] (bool aPressed) {
    mTemperatureController.SetMode(ggController::eModeOff);
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode());
    mWebSockets.UpdateKey(aPressed);
  });
  mPeriphery.mKey.OnReleased([&] () {
    if (mPeriphery.mKey.GetMillisDelta() > 2000) {
      mWifiManager.resetSettings();
      mTemperatureController.SetReference(0.0f);
      mWebSockets.UpdateTemperatureRef(mTemperatureController.GetReference());
    }
  });

  // connect sensor events
  mPeriphery.mSensor.OnStatusChanged([&] (const char* aStatus) {
    mPeriphery.mStatusLED.SetError(!mPeriphery.mSensor.StatusOK());
    mWebSockets.UpdateSensorStatus(aStatus);
  });
  mPeriphery.mSensor.OnTemperatureChanged([&] (float aTemperature) {
    mTemperatureController.SetInput(aTemperature);
    mWebSockets.UpdateTemperature(aTemperature);
  });
  mPeriphery.mSensor.OnHumidityChanged([&] (float aHumidity) {
    mWebSockets.UpdateHumidity(aHumidity);
  });

  // wifi events
  mWiFiConnection.OnConnect([&] () {
    mPeriphery.mStatusLED.SetWarning(false);
    // mWiFiConnection.Print(Serial);
  });
  mWiFiConnection.OnDisconnect([&] () {
    mPeriphery.mStatusLED.SetWarning(true);
  });

  // events from client: control mode, reference temperature
  mWebSockets.OnSetControlMode([&] (int aControlMode) {
    mTemperatureController.SetMode(static_cast<ggController::tMode>(aControlMode));
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode());
  });
  mWebSockets.OnSetTemperatureRef([&] (float aTemperatureRef) {
    mTemperatureController.SetReference(aTemperatureRef);
    mWebSockets.UpdateTemperatureRef(mTemperatureController.GetReference());
  });
  mWebSockets.OnSetOutput([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
  });
}


void loop()
{
  mPeriphery.Run();
  mWebServer.Run();
  mWebSockets.Run();
  mWiFiConnection.Run();
  yield();
}
