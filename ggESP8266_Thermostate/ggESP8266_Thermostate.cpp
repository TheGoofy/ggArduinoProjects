#include <Arduino.h>

// override max number of socket clients (see "WebSocketsServer.h")
// #define WEBSOCKETS_SERVER_CLIENT_MAX (10) // this seems not to work :-( ...

#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <BME280I2C.h>        // https://github.com/finitespace/BME280 (by Tyler Glenn)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>

// PCB version definition (ggPeriphery.h)
// #define M_PCB_VERSION_V1
// #define M_PCB_VERSION_V2
// #define M_PCB_VERSION_V3
// #define M_PCB_VERSION_V4 // actual fabricated PCB (May 2019, installed in Vella)
#define M_PCB_VERSION_V5 // doesn't use RX/TX-pins for SSR-control (serial port still usable for debugging)

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggController.h"
#include "ggValueEEPromString.h"
#include "ggStreams.h"
#include "ggTimerNTP.h"
#include "ggDataLog.h"
#include "ggDisplay.h"


// imoque identification name
const String mHostName = "ESP-SSR-" + String(ESP.getChipId(), HEX);

// file system to use (for webserver and datalogger)
FS* mFileSystem = &SPIFFS; // &LittleFS or &SPIFFS;

// runs AP, if no wifi connection
WiFiManager mWifiManager;
ggWiFiConnection mWiFiConnection;
  
// create web server on port 80
const int mWebServerPort = 80;
ggWebServer mWebServer(mWebServerPort, mFileSystem);

// create web sockets server on port 81
const int mWebSocketsPort = 81;
ggWebSockets mWebSockets(mWebSocketsPort);

// input and ouput configuration
ggPeriphery mPeriphery;

// controls an output based on input and reference value
ggController mTemperatureController;

// display
ggDisplay mDisplay;

// device name
ggValueEEPromString<> mName(mHostName);

// NTP synchronized timer
ggTimerNTP mTimerNTP("ch.pool.ntp.org", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");

// data logging
ggDataLog* mDataLog1H = nullptr;
ggDataLog* mDataLog1D = nullptr;
ggDataLog* mDataLog1W = nullptr;
ggDataLog* mDataLog1M = nullptr;
ggDataLog* mDataLog1Y = nullptr;
ggDataLog* mDataLogMax = nullptr;


void CreateComponents()
{
  // create data loggers with various sampling rates  
  mDataLog1H  = new ggDataLog(2,        60*60,           "/ggData1H.dat",  mFileSystem);
  mDataLog1D  = new ggDataLog(30,       60*60*24,        "/ggData1D.dat",  mFileSystem);
  mDataLog1W  = new ggDataLog(60*5,     60*60*24*7,      "/ggData1W.dat",  mFileSystem);
  mDataLog1M  = new ggDataLog(60*15,    60*60*24*30,     "/ggData1M.dat",  mFileSystem);
  mDataLog1Y  = new ggDataLog(60*60*3,  60*60*24*365,    "/ggData1Y.dat",  mFileSystem);
  mDataLogMax = new ggDataLog(60*60*24, 60*60*24*365*10, "/ggDataMax.dat", mFileSystem);
  /*
  // shorter rollover duration (for debugging)
  mDataLog1H =  new ggDataLog(2,     60*2,     "/ggData1H.dat",  mFileSystem);
  mDataLog1D =  new ggDataLog(10,    60*10,    "/ggData1D.dat",  mFileSystem);
  mDataLog1W =  new ggDataLog(30,    60*30,    "/ggData1W.dat",  mFileSystem);
  mDataLog1M =  new ggDataLog(60,    60*60,    "/ggData1M.dat",  mFileSystem);
  mDataLog1Y =  new ggDataLog(60*5,  60*60*5,  "/ggData1Y.dat",  mFileSystem);
  mDataLogMax = new ggDataLog(60*30, 60*60*30, "/ggDataMax.dat", mFileSystem);
  */
}


void UpdateDisplay()
{
  mDisplay.Clear();
  mDisplay.SetTitle(mName.Get());
  mDisplay.SetText(0, String("SSID: ") + WiFi.SSID());
  mDisplay.SetText(1, String("IP: ") + WiFi.localIP().toString());
  mDisplay.SetText(2, String("Sensor: ") + mPeriphery.mSensor.GetStatus());
  mDisplay.SetText(3, String("Web Sockets: ") + mWebSockets.GetStatus());
}


// wifimanager may be connected before any other comonent (in order to indicate AP-mode)
void ConnectWifiManager()
{
  mWifiManager.setAPCallback([] (WiFiManager* aWiFiManager) {
    mPeriphery.mStatusLED.Begin();
    mPeriphery.mStatusLED.SetWarning(true);
    mDisplay.SetTitle(mWifiManager.getConfigPortalSSID());
    mDisplay.SetText(0, String("IP: ") + WiFi.softAPIP().toString());
    mDisplay.SetText(1, "Access Point for");
    mDisplay.SetText(2, "WiFi configuration");
    mDisplay.SetText(3, "is waiting for you...");
    mDisplay.Run(); // main "loop" is not running
  });
  mWifiManager.setSaveConfigCallback([] () {
    mDisplay.SetText(3, "Config saved!");
    mDisplay.Run(); // main "loop" is not running
    mPeriphery.mStatusLED.SetWarning(false);
  });
}


void ConnectComponents()
{
  // display
  mDisplay.OnConnection([] (bool aConnected) {
    // Maybe update web-sockets? (no need to display this event on the display)
    ggDebug vDebug("mDisplay.OnConnection(...)");
    vDebug.PrintF("aConnected = %d\n", aConnected);
  });

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
    UpdateDisplay();
  });
  mWebSockets.OnClientDisconnect([&] (int aClientID) {
    ggDebug vDebug("mWebSockets.OnClientDisonnect(...)");
    vDebug.PrintF("aClientID = %d\n", aClientID);
    UpdateDisplay();
  });

  // controller event: when output changes, the SSR needs to be switched
  mTemperatureController.OnOutputChanged([&] (float aOutputValue) {
    mPeriphery.mOutputPWM.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
    mDataLog1H->AddOutputValue(aOutputValue);
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
      mName.Set(mHostName);
      mTemperatureController.ResetSettings();
      mWifiManager.resetSettings();
      mDisplay.SetText(3, String("Factory reset..."));
      mDisplay.Run(); // main "loop" is not running
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
    UpdateDisplay();
  });
  mPeriphery.mSensor.OnPressureChanged([&] (float aPressure) {
    mWebSockets.UpdatePressure(aPressure);
    mDataLog1H->AddPressureValue(aPressure);
  });
  mPeriphery.mSensor.OnTemperatureChanged([&] (float aTemperature) {
    mTemperatureController.SetInput(aTemperature);
    mWebSockets.UpdateTemperature(aTemperature);
    mDataLog1H->AddTemperatureSample(aTemperature);
  });
  mPeriphery.mSensor.OnHumidityChanged([&] (float aHumidity) {
    mWebSockets.UpdateHumidity(aHumidity);
    mDataLog1H->AddHumidityValue(aHumidity);
  });

  // wifi events
  mWiFiConnection.OnConnection([&] (bool aConnected) {
    ggDebug vDebug("mWiFiConnection.OnConnection");
    vDebug.PrintF("aConnected = %d\n", aConnected);
    mPeriphery.mStatusLED.SetWarning(!aConnected);
    UpdateDisplay();
  });

  // OTA events
  ArduinoOTA.onStart([] () {
    mPeriphery.mOutputPWM.Set(false); // switch off output (in case OTA fails)
    mPeriphery.mStatusLED.SetOTA(true); // indicate "upload"
    mDisplay.SetText(3, String("OTA update: start"));
    mDisplay.Run(); // main "loop" is not running
  });
  ArduinoOTA.onProgress([] (unsigned int aStep, unsigned int aNumberOfSteps) {
    mDisplay.SetText(3, String("OTA update: ") + (aStep / (aNumberOfSteps / 100)) + "%");
    mDisplay.Run(); // main "loop" is not running
  });
  ArduinoOTA.onEnd([] () {
    mPeriphery.mStatusLED.SetOTA(false);
    mDisplay.SetText(3, String("OTA update: finish"));
    mDisplay.Run(); // main "loop" is not running
  });

  // events from client: control mode, reference temperature, ...
  mWebSockets.OnSetName([&] (const String& aName) {
    mName.Set(aName);
    mWebSockets.UpdateName(mName.Get());
    mDisplay.SetTitle(aName);
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
    mTemperatureController.SetPID(aP, aI, aD);
    mWebSockets.UpdateControlPID(aP, aI, aD);
  });
  mWebSockets.OnSetOutputAnalog([&] (bool aOutputAnalog) {
    mTemperatureController.SetOutputAnalog(aOutputAnalog);
    mWebSockets.UpdateOutputAnalog(mTemperatureController.GetOutputAnalog());
  });
  mWebSockets.OnSetOutput([&] (float aOutputValue) {
    mPeriphery.mOutputPWM.Set(aOutputValue);
    mWebSockets.UpdateOutput(aOutputValue);
    mDataLog1H->AddOutputValue(aOutputValue);
  });

  // web server
  mWebServer.OnDebugStream([] (Stream& aStream) {
    ggStreams vStreams;
    vStreams.push_back(&aStream);
    vStreams.push_back(&Serial);
    ggDebug::SetStream(vStreams);
    ggDebug vDebug("mWebServer.OnDebugStream(...)");
    vDebug.PrintF("mHostName = %s\n", mHostName.c_str());
    mPeriphery.PrintDebug("mPeriphery");
    mTemperatureController.PrintDebug("mTemperatureController");
    ggDebug::SetStream(Serial);
  });
  mWebServer.OnReset([] () {
    mName.Set(mHostName);
    mTemperatureController.ResetSettings();
    mWifiManager.resetSettings();
    mDisplay.SetText(3, String("Factory reset..."));
    mDisplay.Run(); // main "loop" is not running
    ESP.restart();
  });
  mWebServer.OnReboot([] () {
    mDisplay.SetText(3, String("Rebooting..."));
    mDisplay.Run(); // main "loop" is not running
    ESP.restart();
  });
  mWebServer.OnWifiManager([] () {
    mWifiManager.resetSettings();
    mDisplay.SetText(3, String("WiFi reset..."));
    mDisplay.Run(); // main "loop" is not running
    ESP.restart();
  });

  // timer and logging
  mTimerNTP.AddTimer(mDataLog1H->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1H->Write(mTimerNTP.GetTime());
    mDataLog1D->AddValues(*mDataLog1H);
    mDataLog1H->ResetOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1D->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1D->Write(mTimerNTP.GetTime());
    mDataLog1W->AddValues(*mDataLog1D);
    mDataLog1D->ResetOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1W->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1W->Write(mTimerNTP.GetTime());
    mDataLog1M->AddValues(*mDataLog1W);
    mDataLog1W->ResetOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1M->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1M->Write(mTimerNTP.GetTime());
    mDataLog1Y->AddValues(*mDataLog1M);
    mDataLog1M->ResetOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1Y->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1Y->Write(mTimerNTP.GetTime());
    mDataLogMax->AddValues(*mDataLog1Y);
    mDataLog1Y->ResetOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLogMax->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLogMax->Write(mTimerNTP.GetTime());
    mDataLogMax->ResetOnNextAddValue();
  });

  // periodically check (and repair) file system
  mTimerNTP.AddTimer(57, [] (uint32_t aPeriod) {
    GG_DEBUG();
    vDebug.PrintF("Checking the file system...\n");
    vDebug.PrintF("Current NTP time: %s\n", mTimerNTP.GetTime("%d-%m-%Y %H:%M:%S").c_str());
    unsigned long vMicrosStart = micros();
//    for some reason this crashes :-()
//    if (!mFileSystem->check()) { 
//      vDebug.PrintF("check failed\n");
//    }
    bool vResultGC = mFileSystem->gc();
    vDebug.PrintF("GC returned \"%d\"\n", vResultGC);
    unsigned long vMicrosEnd = micros();
    float vDuration = 0.001f * (vMicrosEnd - vMicrosStart);
    vDebug.PrintF("Needed %0.0f ms for checking the file system\n", vDuration);
  });
}


void Run()
{
  mDisplay.Run();
  mPeriphery.Run();
  mTemperatureController.Run();
  mWebServer.Run();
  mWebSockets.Run();
  mWiFiConnection.Run();
  mTimerNTP.Run();
  MDNS.update();
  ArduinoOTA.handle();
  yield();
}


void setup()
{
  // serial communication (for debugging)
  Serial.begin(115200);
  Serial.println("");
  
  GG_DEBUG();

  // initialize eeprom handler
  ggValueEEProm::Begin();
  vDebug.PrintF("Device Name: %s\n", mName.Get().c_str());

  // start the file system
  mFileSystem->begin();

  // init display (for debugging- or user-info)
  mDisplay.Begin();

  // connect to wifi
  ConnectWifiManager();
  mWifiManager.setDebugOutput(false);
  mWifiManager.setConfigPortalTimeout(60); // 1 minute
  mWifiManager.autoConnect(mHostName.c_str());
  vDebug.PrintF("Connected to: %s\n", WiFi.SSID().c_str());
  vDebug.PrintF("IP address: %s\n", WiFi.localIP().toString().c_str());
  mWiFiConnection.Begin();

  // create and connect inputs, outputs, socket-events, ...
  CreateComponents();
  ConnectComponents();

  // NTP timer
  mTimerNTP.Begin();
  vDebug.PrintF("Current NTP time: %s\n", mTimerNTP.GetTime("%d-%m-%Y %H:%M:%S").c_str());

  // configure and start web-server
  mWebServer.Begin();
  vDebug.PrintF("Web server started\n");

  // configure and start web-sockets
  mWebSockets.Begin();
  vDebug.PrintF("Web sockets started\n");

  // start mdns
  MDNS.begin(mHostName.c_str());
  MDNS.addService("http", "tcp", mWebServerPort);
  MDNS.addService("ws", "tcp", mWebSocketsPort);
  vDebug.PrintF("MDNS responder started\n");

  // over the air update
  ArduinoOTA.setHostname(mHostName.c_str());
  ArduinoOTA.begin();
  vDebug.PrintF("OTA service started\n");

  // make sure all status and debug messages are sent before communication gets
  // interrupted, just in case hardware pins are needed for some different use.
  Serial.flush();

  // setup connected hardware
  mPeriphery.Begin();
  mTemperatureController.Begin();
}


void loop()
{
  Run();
}
