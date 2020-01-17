#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <BME280I2C.h>        // https://github.com/finitespace/BME280 (by Tyler Glenn)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

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
#include "ggValueEEPromString.h"


/*
todo:
- "wifimanager" should not block controller operation:
  - option A) wifiManager.setAPCallback(configModeCallback);
  - option B) own wifimanager ...
- in AP-mode also run http-server with controller settings
- scan LAN for connected devices (app for smart-phone)
- which web-interface belongs to which device? "ping" flashing status LED
- data-logging on ESP (multi-time-resolution)
- own PID-controller
- PID auto-tuning algorithm
- adjustable PWM cycle time
- serial stream to rx/tx AND html client console
- SW/HW version
- filter sensor values (iir + median)
*/


const String mHostName = "ESP-SSR-" + String(ESP.getChipId(), HEX);


// runs AP, if no wifi connection
WiFiManager mWifiManager;
ggWiFiConnection mWiFiConnection;
  
// create web server on port 80
const int mWebServerPort = 80;
ggWebServer mWebServer(mWebServerPort);

// create web sockets server on port 81
const int mWebSocketsPort = 81;
ggWebSockets mWebSockets(mWebSocketsPort);

// input and ouput configuration
ggPeriphery mPeriphery;

// controls an output based on input and reference value
ggController mTemperatureController;

// device name
ggValueEEPromString<> mName(mHostName);


void ConnectComponents()
{
  // when a new client is conneted, it needs a complete update
  mWebSockets.OnClientConnect([&] (int aClientID) {
    ggDebug vDebug("mWebSockets.OnClientConnect(...)");
    vDebug.PrintF("aClientID = %d\n", aClientID);
    mWebSockets.UpdateName(mName.Get(), aClientID);
    mWebSockets.UpdateSensorStatus(mPeriphery.mSensor.GetStatus(), aClientID);
    mWebSockets.UpdatePressure(mPeriphery.mSensor.GetPressure(), aClientID);
    mWebSockets.UpdateTemperature(mPeriphery.mSensor.GetTemperature(), aClientID);
    mWebSockets.UpdateHumidity(mPeriphery.mSensor.GetHumidity(), aClientID);
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode(), aClientID);
    mWebSockets.UpdateTemperatureSetPoint(mTemperatureController.GetSetPoint(), aClientID);
    mWebSockets.UpdateHysteresis(mTemperatureController.GetHysteresis(), aClientID);
    mWebSockets.UpdateControlPID(mTemperatureController.GetP(), mTemperatureController.GetI(), mTemperatureController.GetD(), aClientID);
    mWebSockets.UpdateOutputAnalog(mTemperatureController.GetOutputAnalog(), aClientID);
    mWebSockets.UpdateOutput(mTemperatureController.GetOutput(), aClientID);
    mWebSockets.UpdateKey(mPeriphery.mKey.GetPressed(), aClientID);
  });
  mWebSockets.OnClientDisconnect([&] (int aClientID) {
    ggDebug vDebug("mWebSockets.OnClientDisonnect(...)");
    vDebug.PrintF("aClientID = %d\n", aClientID);
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

  // OTA events
  ArduinoOTA.onStart([] () {
    mPeriphery.mStatusLED.SetOTA(true); // indicate "upload"
    mPeriphery.mOutput.Set(false); // switch off output (in case OTA fails)
  });
  ArduinoOTA.onEnd([] () {
    mPeriphery.mStatusLED.SetOTA(false);
  });

  // events from client: control mode, reference temperature, ...
  mWebSockets.OnSetName([&] (const String& aName) {
    mName.Set(aName);
    mWebSockets.UpdateName(mName.Get());
  });
  mWebSockets.OnSetControlMode([&] (int aControlMode) {
    mTemperatureController.SetMode(static_cast<ggController::tMode>(aControlMode));
    mWebSockets.UpdateControlMode(mTemperatureController.GetMode());
  });
  mWebSockets.OnSetTemperatureSetPoint([&] (float aTemperatureSetPoint) {
    mTemperatureController.SetSetPoint(aTemperatureSetPoint);
    mWebSockets.UpdateTemperatureSetPoint(mTemperatureController.GetSetPoint());
  });
  mWebSockets.OnSetHysteresis([&] (float aHysteresis) {
    mTemperatureController.SetHysteresis(aHysteresis);
    mWebSockets.UpdateHysteresis(mTemperatureController.GetHysteresis());
  });
  mWebSockets.OnSetControlPID([&] (float aP, float aI, float aD) {
    ggDebug vDebug("mWebSockets.OnSetControlPID");
    vDebug.PrintF("PID=%f/%f/%f\n", aP, aI, aD);
    mTemperatureController.SetPID(aP, aI, aD);
    mWebSockets.UpdateControlPID(aP, aI, aD);
  });
  mWebSockets.OnSetOutputAnalog([&] (bool aOutputAnalog) {
    mTemperatureController.SetOutputAnalog(aOutputAnalog);
    mWebSockets.UpdateOutputAnalog(mTemperatureController.GetOutputAnalog());
  });
  mWebSockets.OnSetOutput([&] (float aOutputValue) {
    mPeriphery.mOutput.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
  });

  mWebServer.OnDebugStream([] (Stream& aStream) {
    mPeriphery.Print(aStream);
    mTemperatureController.Print(aStream);
  });
  mWebServer.OnReset([] () {
    mTemperatureController.ResetSettings();
    mWifiManager.resetSettings();
  });
  mWebServer.OnReboot([] () {
    ESP.restart();
  });
}


void Run()
{
  mPeriphery.Run();
  mTemperatureController.Run();
  mWebServer.Run();
  mWebSockets.Run();
  mWiFiConnection.Run();
  MDNS.update();
  ArduinoOTA.handle();
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
  
  GG_DEBUG();

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

  // startup eeprom utility class
  ggValueEEProm::Begin();
  Serial.printf("Device Name: %s\n", mName.Get().c_str());

  // connect inputs, outputs, socket-events, ...
  ConnectComponents();

  // configure and start web-server
  mWebServer.Begin();
  Serial.println("Web server started");

  // configure and start web-sockets
  mWebSockets.Begin();
  Serial.println("Web sockets started");

  // start mdns
  MDNS.begin(mHostName.c_str());
  MDNS.addService("http", "tcp", mWebServerPort);
  MDNS.addService("ws", "tcp", mWebSocketsPort);
  Serial.println("MDNS responder started");

  // over the air update
  ArduinoOTA.setHostname(mHostName.c_str());
  ArduinoOTA.begin();
  Serial.println("OTA service started");

  // make sure all status and debug messages are sent before communication gets
  // interrupted, in case hardware pins are needed for some different use.
  Serial.flush();

  // setup connected hardware
  mPeriphery.Begin();
  mTemperatureController.Begin();
}


void loop()
{
  Run();
}
