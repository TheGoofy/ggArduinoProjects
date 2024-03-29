#include <Arduino.h>

// override max number of socket clients (see "WebSocketsServer.h")
// #define WEBSOCKETS_SERVER_CLIENT_MAX (10) // this seems not to work :-( ...

#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <BME280I2C.h>        // https://github.com/finitespace/BME280 (by Tyler Glenn)
#include <U8g2lib.h>          // https://github.com/olikraus/u8g2 by oliver
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>

#define M_DEBUGGING false

// PCB version definition (ggPeriphery.h)
// #define M_PCB_VERSION_V1
// #define M_PCB_VERSION_V2
// #define M_PCB_VERSION_V3
// #define M_PCB_VERSION_V4 // actual fabricated PCB (May 2019, installed in Vella)
#define M_PCB_VERSION_V5 // doesn't use RX/TX-pins for SSR-control (serial port still usable for debugging)

#define M_VERSION_SW "v1.0.1"

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggController.h"
#include "ggValueEEPromString.h"
#include "ggStreams.h"
#include "ggTimerNTP.h"
#include "ggDataLog.h"


// imoque identification name
const String mHostName = "ESP-SSR-" + String(ESP.getChipId(), HEX);
const String mHostPassword = "ESP-1234";

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

// up-time
unsigned long long mMillisUpTime = 0;


void UpdateUpTime()
{
  static unsigned long vMillisLast = 0;
  unsigned long vMillis = millis();
  unsigned long vMillisDelta = vMillis - vMillisLast;
  mMillisUpTime += vMillisDelta;
  vMillisLast = vMillis;
}


void CreateComponents()
{
  // create data loggers with various sampling rates  
  mDataLog1H  = new ggDataLog(15,       60*60,           "/ggData1H.dat",  mFileSystem);
  mDataLog1D  = new ggDataLog(60,       60*60*24,        "/ggData1D.dat",  mFileSystem);
  mDataLog1W  = new ggDataLog(60*5,     60*60*24*7,      "/ggData1W.dat",  mFileSystem);
  mDataLog1M  = new ggDataLog(60*15,    60*60*24*30,     "/ggData1M.dat",  mFileSystem);
  mDataLog1Y  = new ggDataLog(60*60*3,  60*60*24*365,    "/ggData1Y.dat",  mFileSystem);
  mDataLogMax = new ggDataLog(60*60*24, 60*60*24*365*10, "/ggDataMax.dat", mFileSystem);
  /*
  // shorter file rollover duration (for debugging)
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
  mPeriphery.mDisplay.Clear();
  mPeriphery.mDisplay.SetTitle(mName.Get());
  mPeriphery.mDisplay.SetText(0, String("SSID: ") + WiFi.SSID());
  mPeriphery.mDisplay.SetText(1, String("IP: ") + WiFi.localIP().toString());
  mPeriphery.mDisplay.SetText(2, String("Sensor: ") + mPeriphery.mSensor.GetStatus());
  mPeriphery.mDisplay.SetText(3, String("Web Sockets: ") + mWebSockets.GetStatus());
}


void ResetAll()
{
  mPeriphery.mDisplay.SetText(3, String("Reset all..."));
  mPeriphery.mDisplay.Run(); // main "loop" is not running
  mName.Set(mHostName);
  mDataLog1H->ResetAll();
  mDataLog1D->ResetAll();
  mDataLog1W->ResetAll();
  mDataLog1M->ResetAll();
  mDataLog1Y->ResetAll();
  mDataLogMax->ResetAll();
  mTemperatureController.ResetSettings();
  mWifiManager.resetSettings();
}


// wifimanager may be connected before any other comonent (in order to indicate AP-mode)
void ConnectWifiManager()
{
  mWifiManager.setAPCallback([] (WiFiManager* aWiFiManager) {
    mPeriphery.mStatusLED.Begin();
    mPeriphery.mStatusLED.SetWarning(true);
    mPeriphery.mDisplay.Begin();
    mPeriphery.mDisplay.SetTitle(mWifiManager.getConfigPortalSSID());
    mPeriphery.mDisplay.SetText(0, String("IP: ") + WiFi.softAPIP().toString());
    mPeriphery.mDisplay.SetText(1, "Access Point for");
    mPeriphery.mDisplay.SetText(2, "WiFi configuration");
    mPeriphery.mDisplay.SetText(3, "is waiting for you...");
    mPeriphery.mDisplay.Run(); // main "loop" is not running
  });
  mWifiManager.setSaveConfigCallback([] () {
    mPeriphery.mDisplay.SetText(3, "Config saved!");
    mPeriphery.mDisplay.Run(); // main "loop" is not running
    mPeriphery.mStatusLED.SetWarning(false);
  });
}


void ConnectComponents()
{
  // display
  mPeriphery.mDisplay.OnConnection([] (bool aConnected) {
    // Maybe update web-sockets? (no need to display this event on the display)
    GG_DEBUG_BLOCK("mPeriphery.mDisplay.OnConnection(...)");
    GG_DEBUG_PRINTF("aConnected = %d\n", aConnected);
  });

  // when a new client is conneted, it needs a complete update
  mWebSockets.OnClientConnect([&] (int aClientID) {
    GG_DEBUG_BLOCK("mWebSockets.OnClientConnect(...)");
    GG_DEBUG_PRINTF("aClientID = %d\n", aClientID);
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
    GG_DEBUG_BLOCK("mWebSockets.OnClientDisonnect(...)");
    GG_DEBUG_PRINTF("aClientID = %d\n", aClientID);
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
      ResetAll();
      delay(1000);
      ESP.restart();
    }
  });

  // connect sensor events
  mPeriphery.mSensor.OnStatusChanged([&] (const char* aStatus) {
    GG_DEBUG_BLOCK("mPeriphery.mSensor.OnStatusChanged");
    GG_DEBUG_PRINTF("aStatus = %s\n", aStatus);
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
    GG_DEBUG_BLOCK("mWiFiConnection.OnConnection");
    GG_DEBUG_PRINTF("aConnected = %d\n", aConnected);
    mPeriphery.mStatusLED.SetWarning(!aConnected);
    UpdateDisplay();
  });

  // OTA events
  ArduinoOTA.onStart([] () {
    mPeriphery.mOutputPWM.Set(false); // switch off output (in case OTA fails)
    mPeriphery.mStatusLED.SetOTA(true); // indicate "upload"
    mPeriphery.mDisplay.SetText(3, String("OTA update: start"));
    mPeriphery.mDisplay.Run(); // main "loop" is not running
  });
  ArduinoOTA.onProgress([] (unsigned int aStep, unsigned int aNumberOfSteps) {
    mPeriphery.mDisplay.SetText(3, String("OTA update: ") + (aStep / (aNumberOfSteps / 100)) + "%");
    mPeriphery.mDisplay.Run(); // main "loop" is not running
  });
  ArduinoOTA.onEnd([] () {
    mPeriphery.mStatusLED.SetOTA(false);
    mPeriphery.mDisplay.SetText(3, String("OTA update: done"));
    mPeriphery.mDisplay.Run(); // main "loop" is not running
  });

  // events from client: control mode, reference temperature, ...
  mWebSockets.OnSetName([&] (const String& aName) {
    mName.Set(aName);
    mWebSockets.UpdateName(mName.Get());
    mPeriphery.mDisplay.SetTitle(aName);
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
    {
      ggDebug vDebug("mWebServer.OnDebugStream(...)");
      vDebug.PrintF("Version SW = %s (%s)\n", M_VERSION_SW, __DATE__);
      vDebug.PrintF("Version HW = %s\n", M_VERSION_HW);
      vDebug.PrintF("mHostName = %s\n", mHostName.c_str());
      vDebug.PrintF("mHostPassword = %s\n", mHostPassword.isEmpty() ? "off" : "required");
      vDebug.PrintF("mMillisUpTime = %llu (%.1f Days)\n", mMillisUpTime, mMillisUpTime/(1000.0f*60.0f*60.0f*24.0f));
      mPeriphery.PrintDebug("mPeriphery");
      mWiFiConnection.PrintDebug("mWiFiConnection");
      mTimerNTP.PrintDebug("mTimerNTP");
      mTemperatureController.PrintDebug("mTemperatureController");
    }
    ggDebug::SetStream(Serial);
  });
  mWebServer.OnResetAll([] () {
    ResetAll();
    delay(1000);
    ESP.restart();
  });
  mWebServer.OnResetWifi([] () {
    mPeriphery.mDisplay.SetText(3, String("Reset WiFi..."));
    mPeriphery.mDisplay.Run(); // main "loop" is not running
    mWifiManager.resetSettings();
    delay(1000);
    ESP.restart();
  });
  mWebServer.OnReboot([] () {
    mPeriphery.mDisplay.SetText(3, String("Rebooting..."));
    mPeriphery.mDisplay.Run(); // main "loop" is not running
    delay(1000);
    ESP.restart();
  });

  // timer and logging
  mTimerNTP.AddTimer(mDataLog1H->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1H->Write(mTimerNTP.GetTimeT());
    mDataLog1D->AddValues(*mDataLog1H);
    mDataLog1H->ResetAveragesOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1D->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1D->Write(mTimerNTP.GetTimeT());
    mDataLog1W->AddValues(*mDataLog1D);
    mDataLog1D->ResetAveragesOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1W->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1W->Write(mTimerNTP.GetTimeT());
    mDataLog1M->AddValues(*mDataLog1W);
    mDataLog1W->ResetAveragesOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1M->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1M->Write(mTimerNTP.GetTimeT());
    mDataLog1Y->AddValues(*mDataLog1M);
    mDataLog1M->ResetAveragesOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLog1Y->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLog1Y->Write(mTimerNTP.GetTimeT());
    mDataLogMax->AddValues(*mDataLog1Y);
    mDataLog1Y->ResetAveragesOnNextAddValue();
  });
  mTimerNTP.AddTimer(mDataLogMax->GetPeriod(), [] (uint32_t aPeriod) {
    mDataLogMax->Write(mTimerNTP.GetTimeT());
    mDataLogMax->ResetAveragesOnNextAddValue();
  });

  // periodically check (and repair) file system
  mTimerNTP.AddTimer(57, [] (uint32_t aPeriod) {
    GG_DEBUG();
    GG_DEBUG_PRINTF("Checking the file system...\n");
    GG_DEBUG_PRINTF("Current NTP time: %s\n", mTimerNTP.GetTime("%d-%m-%Y %H:%M:%S").c_str());
    unsigned long vMicrosStart = micros();
//    for some reason this crashes :-()
//    if (!mFileSystem->check()) { 
//      GG_DEBUG_PRINTF("check failed\n");
//    }
    bool vResultGC = mFileSystem->gc();
    GG_DEBUG_PRINTF("GC returned \"%d\"\n", vResultGC);
    unsigned long vMicrosEnd = micros();
    float vDuration = 0.001f * (vMicrosEnd - vMicrosStart);
    GG_DEBUG_PRINTF("Needed %0.0f ms for checking the file system\n", vDuration);
  });
}


void Run()
{
  mPeriphery.Run();
  mWebServer.Run();
  mWebSockets.Run();
  mWiFiConnection.Run();
  mTimerNTP.Run();
  MDNS.update();
  ArduinoOTA.handle();
  UpdateUpTime();
  yield();
}


void setup()
{
  // serial communication (for debugging)
  #if M_DEBUGGING
  Serial.begin(115200);
  Serial.println("");
  #endif
  
  GG_DEBUG();

  // initialize eeprom handler
  ggValueEEProm::Begin();
  GG_DEBUG_PRINTF("Device Name: %s\n", mName.Get().c_str());

  // start the file system
  mFileSystem->begin();

  // early init display (for debugging- or user-info)
  mPeriphery.mDisplay.Begin();

  // connect to wifi
  ConnectWifiManager();
  mWifiManager.setDebugOutput(M_DEBUGGING);
  mWifiManager.setConfigPortalTimeout(60); // 1 minute
  mWifiManager.autoConnect(mHostName.c_str(), mHostPassword.c_str());
  GG_DEBUG_PRINTF("Connected to: %s\n", WiFi.SSID().c_str());
  GG_DEBUG_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
  mWiFiConnection.Begin();

  // create and connect inputs, outputs, socket-events, ...
  CreateComponents();
  ConnectComponents();

  // NTP timer
  mTimerNTP.Begin();
  GG_DEBUG_PRINTF("Current NTP time: %s\n", mTimerNTP.GetTime("%d-%m-%Y %H:%M:%S").c_str());

  // configure and start web-server
  mWebServer.Begin();
  GG_DEBUG_PRINTF("Web server started\n");

  // configure and start web-sockets
  mWebSockets.Begin();
  GG_DEBUG_PRINTF("Web sockets started\n");

  // start mdns
  MDNS.begin(mHostName.c_str());
  MDNS.addService("http", "tcp", mWebServerPort);
  MDNS.addService("ws", "tcp", mWebSocketsPort);
  GG_DEBUG_PRINTF("MDNS responder started\n");

  // over the air update
  ArduinoOTA.setHostname(mHostName.c_str());
  ArduinoOTA.setPassword(mHostPassword.c_str());
  ArduinoOTA.begin();
  GG_DEBUG_PRINTF("OTA service started\n");

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
