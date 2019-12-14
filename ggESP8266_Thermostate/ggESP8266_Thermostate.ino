#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <BME280I2C.h>        // https://github.com/finitespace/BME280 (by Tyler Glenn)

// PCB version definition (ggPeriphery.h)
// #define M_PCB_VERSION_V1
// #define M_PCB_VERSION_V2
// #define M_PCB_VERSION_V3
// #define M_PCB_VERSION_V4 // actual fabricated PCB (May 2019)
#define M_PCB_VERSION_V5 // doesn't use RX/TX-pins for SSR-control (serial port still usable for debugging)

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggController.h"


/*
todo:
- "wifimanager" should not block controller operation:
  - option A) wifiManager.setAPCallback(configModeCallback);
  - option B) own wifimanager ...
- indicate AP-mode (wifimanager) with long-blinking status LED
- in AP-mode also run http-server with controller settings
- OTA update mode:
  - option A) after startup (reset) for a limited amout of time (safer)
  - option B) activate via web-interface (easyer remote update)
- use <Ticker.h> for PWM output
- which web-interface belongs to which device? "ping" flashing status LED
- scan LAN for connected devices (app for smart-phone)
- custom device name "bedroom, etc..."
*/

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


void ConnectComponents()
{
  // when a new client is conneted, it needs a complete update
  mWebSockets.OnClientConnect([&] (int aClientID) {
    ggDebug vDebug("mWebSockets.OnClientConnect");
    vDebug.PrintF("aClientID = %d\n", aClientID);
    mWebSockets.UpdateSensorStatus(mPeriphery.mSensor.GetStatus(), aClientID);
    mWebSockets.UpdatePressure(mPeriphery.mSensor.GetPressure(), aClientID);
    mWebSockets.UpdateTemperature(mPeriphery.mSensor.GetTemperature(), aClientID);
    mWebSockets.UpdateHumidity(mPeriphery.mSensor.GetHumidity(), aClientID);
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode(), aClientID);
    mWebSockets.UpdateTemperatureRef(mTemperatureController.GetReference(), aClientID);
    mWebSockets.UpdateHysteresis(mTemperatureController.GetHysteresis(), aClientID);
    mWebSockets.UpdateOutputAnalog(mTemperatureController.GetOutputAnalog(), aClientID);
    mWebSockets.UpdateOutput(mTemperatureController.GetOutput(), aClientID);
    mWebSockets.UpdateKey(mPeriphery.mKey.GetPressed(), aClientID);
  });
  
  // controller event: when output changes, the SSR needs to be switched
  mTemperatureController.OnOutputChanged([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
  });

  // when button "key" is pressed we switch the SSR manually
  mPeriphery.mKey.OnChanged([&] (bool aPressed) {
    mWebSockets.UpdateKey(aPressed);
  });
  mPeriphery.mKey.OnPressed([&] () {
    mTemperatureController.SetMode(mTemperatureController.GetMode() != ggController::eModeOff ? ggController::eModeOff : ggController::eModeOn);
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode());
  });
  mPeriphery.mKey.OnPressedFor(5000, [&] () {
    mPeriphery.mStatusLED.SetWarning(true);
  });
  mPeriphery.mKey.OnReleased([&] () {
    if (mPeriphery.mKey.GetMillisDelta() > 5000) {
      mTemperatureController.ResetSettings();
      mWifiManager.resetSettings();
      ESP.restart();
    }
  });

  // connect sensor events
  mPeriphery.mSensor.OnStatusChanged([&] (const char* aStatus) {
    ggDebug vDebug("mPeriphery.mSensor.OnStatusChanged");
    vDebug.PrintF("aStatus = %s\n", aStatus);
    mTemperatureController.SetInputValid(mPeriphery.mSensor.StatusOK());
    mPeriphery.mStatusLED.SetError(!mPeriphery.mSensor.StatusOK());
    mWebSockets.UpdateSensorStatus(aStatus);
  });
  mPeriphery.mSensor.OnPressureChanged([&] (float aPressure) {
    mWebSockets.UpdatePressure(aPressure);
  });
  mPeriphery.mSensor.OnTemperatureChanged([&] (float aTemperature) {
    // ggDebug vDebug("mPeriphery.mSensor.OnTemperatureChanged");
    // vDebug.PrintF("aTemperature = %0.3f\n", aTemperature);
    mTemperatureController.SetInput(aTemperature);
    mWebSockets.UpdateTemperature(aTemperature);
  });
  mPeriphery.mSensor.OnHumidityChanged([&] (float aHumidity) {
    mWebSockets.UpdateHumidity(aHumidity);
  });

  // wifi events
  mWiFiConnection.OnConnect([&] () {
    ggDebug vDebug("mWiFiConnection.OnConnect");
    mPeriphery.mStatusLED.SetWarning(false);
  });
  mWiFiConnection.OnDisconnect([&] () {
    ggDebug vDebug("mWiFiConnection.OnDisconnect");
    mPeriphery.mStatusLED.SetWarning(true);
  });

  // events from client: control mode, reference temperature, ...
  mWebSockets.OnSetControlMode([&] (int aControlMode) {
    mTemperatureController.SetMode(static_cast<ggController::tMode>(aControlMode));
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode());
  });
  mWebSockets.OnSetTemperatureRef([&] (float aTemperatureRef) {
    mTemperatureController.SetReference(aTemperatureRef);
    mWebSockets.UpdateTemperatureRef(mTemperatureController.GetReference());
  });
  mWebSockets.OnSetHysteresis([&] (float aHysteresis) {
    mTemperatureController.SetHysteresis(aHysteresis);
    mWebSockets.UpdateHysteresis(mTemperatureController.GetHysteresis());
  });
  mWebSockets.OnSetOutputAnalog([&] (bool aOutputAnalog) {
    mTemperatureController.SetOutputAnalog(aOutputAnalog);
    mWebSockets.UpdateOutputAnalog(mTemperatureController.GetOutputAnalog());
  });
  mWebSockets.OnSetOutput([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
  });
}


void Run()
{
  mPeriphery.Run();
  // mWebServer.Run();
  mWebSockets.Run();
  mWiFiConnection.Run();
  yield();
}


void WifiManagerConfigPortalStart(WiFiManager* aWiFiManager)
{
  mPeriphery.mStatusLED.Begin();
  mPeriphery.mStatusLED.SetWarning(true);
}


void WifiManagerConfigPortalEnd()
{
  mPeriphery.mStatusLED.SetWarning(false);
}


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");
  
  Serial.printf("A - constructing string s1 with argument \"%s\"\n", __FUNCTION__);
  Serial.flush();
  
  String s1(__FUNCTION__);

  Serial.printf("B - string s1 value is \"%s\"\n", s1.c_str());
  Serial.flush();

  Serial.printf("C - constructing string s2 with argument \"%s\"\n", __PRETTY_FUNCTION__);
  Serial.flush();
  
  // String s2(__PRETTY_FUNCTION__);

  // Serial.printf("D - string s2 value is \"%s\"\n", s2.c_str());
  // Serial.flush();
  
  // ggDebug vDebug(__PRETTY_FUNCTION__);

  // startup eeprom utility class
  ggValueEEProm::Begin();

  // connect to wifi
  mWifiManager.setDebugOutput(true);
  mWifiManager.setAPCallback(WifiManagerConfigPortalStart);
  mWifiManager.setSaveConfigCallback(WifiManagerConfigPortalEnd);
  mWifiManager.setConfigPortalTimeout(60); // 1 minute
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
  
  // connect inputs, outputs, socket-events, ...
  ConnectComponents();
  
  // setup connected hardware
  mPeriphery.Begin();
}


void loop()
{
  Run();
}
