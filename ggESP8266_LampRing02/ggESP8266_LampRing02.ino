#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define M_DEBUGGING false

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggData.h"
#include "ggNullStream.h"
#include "ggStreams.h"
#include "ggTimer.h"


// unique identification name
const String mHostName = "ESP-Lamp-" + String(ESP.getChipId(), HEX);


// file system to use (for webserver and datalogger)
FS* mFileSystem = &SPIFFS; // &LittleFS or &SPIFFS;


// ports
const int mWebServerPort = 80;
const int mWebSocketsPort = 81;


#ifdef M_DEBUGGING
Stream& mDebugStream(Serial);
#else
ggNullStream mDebugStream;
#endif


WiFiManager& WiFiMgr()
{
  static WiFiManager* vWifiManager = nullptr;
  if (vWifiManager == nullptr) vWifiManager = new WiFiManager();
  return *vWifiManager;
}


ggWebSockets& WebSockets()
{
  static ggWebSockets* vWebSockets = nullptr;
  if (vWebSockets == nullptr) vWebSockets = new ggWebSockets(mWebSocketsPort);
  return *vWebSockets;
}


ggPeriphery& Periphery()
{
  static ggPeriphery* vPeriphery = nullptr;
  if (vPeriphery == nullptr) vPeriphery = new ggPeriphery;
  return *vPeriphery;
}


ggTimer& DisplayTimer()
{
  static ggTimer* vTimer = nullptr;
  if (vTimer == nullptr) vTimer = new ggTimer(30.0f); // timeout in 30 seconds
  return *vTimer;
}


ggTimer& EditTimer()
{
  static ggTimer* vTimer = nullptr;
  if (vTimer == nullptr) vTimer = new ggTimer(10.0f); // timeout in 10 seconds
  return *vTimer;
}


ggWiFiConnection& WiFiConnection()
{
  static ggWiFiConnection* vWiFiConnection = nullptr;
  if (vWiFiConnection == nullptr) vWiFiConnection = new ggWiFiConnection();
  return *vWiFiConnection;
}


ggWebServer& WebServer()
{
  static ggWebServer* vWebServer = nullptr;
  if (vWebServer == nullptr) vWebServer = new ggWebServer(mWebServerPort, mFileSystem);
  return *vWebServer;
}


ggData& Data()
{
  static ggData* vData = nullptr;
  if (vData == nullptr) {
    GG_DEBUG();
    GG_DEBUG_PRINTF("allocating data\n");
    vData = new ggData(mHostName, 1);
  }
  return *vData;
}


void UpdateDisplay()
{
  GG_DEBUG();
  Periphery().mDisplay.Clear();
  Periphery().mDisplay.SetTitle(Data().mName.Get());
  Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
}


void DataSetChannelBrightness(const float& aB0, const float& aB1, const float& aB2, const float& aB3, const float& aB4, const float& aB5)
{
  GG_DEBUG();
  ggValueEEProm::cLazyWriter vLazyWriter;
  Data().mCurrentScene.mBrightnesses[0] = aB0;
  Data().mCurrentScene.mBrightnesses[1] = aB1;
  Data().mCurrentScene.mBrightnesses[2] = aB2;
  Data().mCurrentScene.mBrightnesses[3] = aB3;
  Data().mCurrentScene.mBrightnesses[4] = aB4;
  Data().mCurrentScene.mBrightnesses[5] = aB5;
}


void DataChangeBrightness(const float& aBrightnessDelta)
{
  GG_DEBUG();
  ggValueEEProm::cLazyWriter vLazyWriter;
  for (ggData::tBrightness& vBrightness : Data().mCurrentScene.mBrightnesses) {
    float vBrightnessNew = vBrightness + aBrightnessDelta;
    vBrightness = ggClamp(vBrightnessNew, 0.0f, 1.0f);
  }
}


void PeripheryLEDCenterSetChannelBrightness()
{
  GG_DEBUG();
  Periphery().mLEDCenter.ForEachChannel([] (int aChannel) {
    Periphery().mLEDCenter.SetChannelBrightness(aChannel, Data().mCurrentScene.mBrightnesses[aChannel]);
  });
}


void WebSocketsUpdateChannelBrightness(int aClientID = -1)
{
  GG_DEBUG();
  WebSockets().UpdateChannelBrightness(Data().mCurrentScene.mBrightnesses[0],
                                       Data().mCurrentScene.mBrightnesses[1],
                                       Data().mCurrentScene.mBrightnesses[2],
                                       Data().mCurrentScene.mBrightnesses[3],
                                       Data().mCurrentScene.mBrightnesses[4],
                                       Data().mCurrentScene.mBrightnesses[5],
                                       aClientID);
}


void WebSocketsUpdateRingColorHSV(int aClientID = -1)
{
  GG_DEBUG();
  const ggColor::cHSV& vHSV0(Periphery().mLEDRing.GetColorHSV(ggLocations::eAL));
  const ggColor::cHSV& vHSV1(Periphery().mLEDRing.GetColorHSV(ggLocations::eAR));
  const ggColor::cHSV& vHSV2(Periphery().mLEDRing.GetColorHSV(ggLocations::eBL));
  const ggColor::cHSV& vHSV3(Periphery().mLEDRing.GetColorHSV(ggLocations::eBR));
  WebSockets().UpdateRingColorHSV(vHSV0.mH, vHSV0.mS, vHSV0.mV,
                                  vHSV1.mH, vHSV1.mS, vHSV1.mV,
                                  vHSV2.mH, vHSV2.mS, vHSV2.mV,
                                  vHSV3.mH, vHSV3.mS, vHSV3.mV,
                                  aClientID);
}


struct ggState {

  typedef enum tEnum {
    eOff,
    eOn,
    eEditColorChannel0,
    eEditColorChannel1,
    eEditColorChannel2,
    eEditReset,
    eResetWiFi
  };

  static int GetColorChannelIndex(tEnum aState) {
    switch (aState) {
      case eEditColorChannel0: return 0;
      case eEditColorChannel1: return 1;
      case eEditColorChannel2: return 2;
      default: return 0;
    }
  }

};


struct ggEvent {
  typedef enum tEnum {
    eClick,
    eClickLong,
    eTimeout
  };
};


class ggLampState {

public:

  ggState::tEnum mState;
  
  typedef std::function<void (ggState::tEnum aState)> tStateFunc;
  tStateFunc mStateFunc;
  
  ggLampState()
  : mState(ggState::eOff),
    mStateFunc(nullptr) {
  }

  ggState::tEnum GetState() const {
    return mState;
  }

  void SetState(ggState::tEnum aState) {
    if (mState != aState) {
      mState = aState;
      if (mStateFunc) mStateFunc(mState);
    }
  }
  
  void OnState(tStateFunc aStateFunc) {
    mStateFunc = aStateFunc;
  }

  void HandleEvent(ggEvent::tEnum aEvent) {
    switch (mState) {
      case ggState::eOff:
        if (aEvent == ggEvent::eClick) SetState(ggState::eOn);
        break;
      case ggState::eOn:
        if (aEvent == ggEvent::eClick) SetState(ggState::eOff);
        if (aEvent == ggEvent::eClickLong) SetState(ggState::eEditColorChannel0);
        break;
      case ggState::eEditColorChannel0:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditColorChannel1);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditColorChannel1:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditColorChannel2);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditColorChannel2:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditReset);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditReset:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditColorChannel0);
        if (aEvent == ggEvent::eClickLong) SetState(ggState::eResetWiFi); // potentially this will reboot the system ...
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eResetWiFi:
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      default:
        break;
    }
  }
};


ggLampState mLampState;


// wifimanager may be connected before any other comonent (in order to indicate AP-mode)
void ConnectWifiManager()
{
  WiFiMgr().setAPCallback([] (WiFiManager* aWiFiManager) {
    Periphery().mDisplay.SetOn(true);
    Periphery().mDisplay.SetTitle(WiFiMgr().getConfigPortalSSID());
    Periphery().mDisplay.SetText(0, "WiFi config...");
    Periphery().mDisplay.Run(); // main "loop" is not running
  });
  WiFiMgr().setSaveConfigCallback([] () {
    Periphery().mDisplay.SetText(0, "Config saved!");
    Periphery().mDisplay.Run(); // main "loop" is not running
  });
}


void ConnectComponents()
{
  // mode
  mLampState.OnState([&] (ggState::tEnum aState) {
    switch (aState) {
      case ggState::eOff:
        Data().mOn = false;
        Periphery().SetOff();
        Periphery().mLEDCenter.SetChannelBrightness(0.0f);
        Periphery().mDisplay.SetOn(false);
        WebSockets().UpdateOn(false);
        break;
      case ggState::eOn:
        Data().mOn = true;
        Periphery().SetOn();
        PeripheryLEDCenterSetChannelBrightness();
        Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
        WebSockets().UpdateOn(true);
        break;
      case ggState::eEditColorChannel0:
        Periphery().mDisplay.SetText(0, "Color Hue");
        Periphery().mLEDRing.DisplayColorChannel(0);
        EditTimer().Start();
        break;
      case ggState::eEditColorChannel1:
        Periphery().mDisplay.SetText(0, "Saturaton");
        Periphery().mLEDRing.DisplayColorChannel(1);
        EditTimer().Start();
        break;
      case ggState::eEditColorChannel2:
        Periphery().mDisplay.SetText(0, "Brightness");
        Periphery().mLEDRing.DisplayColorChannel(2);
        EditTimer().Start();
        break;
      case ggState::eEditReset:
        Periphery().mDisplay.SetText(0, "Reset");
        Periphery().mLEDRing.DisplayColor(ggColor::cRGB::Orange());
        EditTimer().Start();
        break;
      case ggState::eResetWiFi:
        {
          GG_DEBUG_BLOCK("ggState::eResetWiFi");
          Periphery().mLEDRing.DisplayColor(ggColor::cRGB::Red());
          Periphery().mDisplay.SetText(0, String("Reset WiFi..."));
          Periphery().mDisplay.Run(); // main "loop" is not running
          WiFiMgr().resetSettings();
          delay(1000);
        }
        ESP.restart();
        break;
    }
  });

  // click
  static bool vLongPressed = false;
  
  // clicking "on/off"
  Periphery().mButton.OnReleased([&] () {
    if (!vLongPressed) mLampState.HandleEvent(ggEvent::eClick);
    else vLongPressed = false;
    if (mLampState.GetState() != ggState::eOff) {
      Periphery().mDisplay.SetOn(true);
      DisplayTimer().Start();
    }
  });

  // long press changes mode
  Periphery().mButton.OnPressedFor(2000, [&] () {
    DisplayTimer().Start();
    Periphery().mDisplay.SetOn(true);
    mLampState.HandleEvent(ggEvent::eClickLong);
    vLongPressed = true;
  });

  // rotary encoder signal (4 increments per tick and 20 ticks per revolution)
  Periphery().mEncoder.OnValueChangedDelta([&] (long aValueDelta) {
    DisplayTimer().Start();
    Periphery().mDisplay.SetOn(true);
    switch (mLampState.GetState()) {
      case ggState::eOn:
        DataChangeBrightness(0.25f * 0.05f * aValueDelta);
        PeripheryLEDCenterSetChannelBrightness();
        WebSocketsUpdateChannelBrightness();
        EditTimer().Start();
        break;
      case ggState::eEditColorChannel0:
      case ggState::eEditColorChannel1:
      case ggState::eEditColorChannel2:
        Periphery().mLEDRing.ChangeColorChannel(ggState::GetColorChannelIndex(mLampState.GetState()), aValueDelta);
        WebSocketsUpdateRingColorHSV();
        EditTimer().Start();
        break;
    }
  });

  // display
  Periphery().mDisplay.OnConnection([] (bool aConnected) {
    if (aConnected) UpdateDisplay();
  });

  // switch off display, when not in use
  DisplayTimer().OnTimeOut([&] () {
    Periphery().mDisplay.SetOn(false);
  });

  // switch back to normal after a while of no user inputs
  EditTimer().OnTimeOut([&] () {
    mLampState.HandleEvent(ggEvent::eTimeout);
  });

  // wifi events
  WiFiConnection().OnConnection([&] (bool aConnected) {
    GG_DEBUG_BLOCK("WiFiConnection().OnConnection()");
    GG_DEBUG_PRINTF("aConnected = %d\n", aConnected);
    UpdateDisplay();
  });

  // events from websocket clients ...
  WebSockets().OnClientConnect([&] (int aClientID) { // need to update all sockets
    GG_DEBUG_BLOCK("WebSockets().OnClientConnect(...)");
    GG_DEBUG_PRINTF("aClientID = %d\n", aClientID);
    WebSockets().UpdateName(Data().mName.Get(), aClientID);
    WebSockets().UpdateOn(Data().mOn.Get(), aClientID);
    WebSocketsUpdateChannelBrightness(aClientID);
    WebSocketsUpdateRingColorHSV(aClientID);
  });
  WebSockets().OnClientDisconnect([&] (int aClientID) {
    GG_DEBUG_BLOCK("WebSockets().OnClientDisonnect(...)");
    GG_DEBUG_PRINTF("aClientID = %d\n", aClientID);
  });
  WebSockets().OnSetName([&] (const String& aName) {
    GG_DEBUG_BLOCK("WebSockets().OnSetName(...)");
    GG_DEBUG_PRINTF("aName = %s\n", aName.c_str());
    Data().mName.Set(aName);
    WebSockets().UpdateName(aName);
    Periphery().mDisplay.SetTitle(aName);
  });
  WebSockets().OnSetOn([&] (bool aOn) {
    mLampState.SetState(aOn ? ggState::eOn : ggState::eOff);
  });
  WebSockets().OnSetChannelBrightness([&] (float aB0, float aB1, float aB2, float aB3, float aB4, float aB5) {
    DataSetChannelBrightness(aB0, aB1, aB2, aB3, aB4, aB5);
    PeripheryLEDCenterSetChannelBrightness();
    WebSocketsUpdateChannelBrightness();
  });
  WebSockets().OnSetRingColorHSV([&] (uint8_t aH, uint8_t aS, uint8_t aV, uint8_t aLocations) {
    Periphery().mLEDRing.SetColor(ggColor::cHSV(aH, aS, aV), (ggLocations)aLocations);
    WebSocketsUpdateRingColorHSV();
  });
  
  // web server
  WebServer().OnDebugStream([] (Stream& aStream) {
    ggStreams vStreams;
    vStreams.push_back(&aStream);
    vStreams.push_back(&mDebugStream);
    ggDebug::SetStream(vStreams);
    ggDebug vDebug("mWebServer.OnDebugStream(...)");
    vDebug.PrintF("mHostName = %s\n", mHostName.c_str());
    Periphery().PrintDebug("mPeriphery");
    ggDebug::SetStream(mDebugStream);
  });
  WebServer().OnResetAll([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnResetAll(...)");
      Periphery().mDisplay.SetText(0, String("Reset All..."));
      Periphery().mDisplay.Run(); // main "loop" is not running
      Data().mName.Set(mHostName);
      Periphery().ResetSettings();
      WiFiMgr().resetSettings();
      delay(1000);
    }
    ESP.restart();
  });
  WebServer().OnResetWifi([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnResetWifi(...)");
      Periphery().mDisplay.SetText(0, String("Reset WiFi..."));
      Periphery().mDisplay.Run(); // main "loop" is not running
      WiFiMgr().resetSettings();
      delay(1000);
    }
    ESP.restart();
  });
  WebServer().OnReboot([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnReboot(...)");
      Periphery().mDisplay.SetText(0, String("Rebooting..."));
      Periphery().mDisplay.Run(); // main "loop" is not running
      delay(1000);
    }
    ESP.restart();
  });

  // OTA status display
  static ggColor::cRGB vColorProgress(80,30,0);
  static ggColor::cRGB vColorProgressBackground(0,0,0);
  static ggColor::cRGB vColorSuccess(20,80,0);
  static ggColor::cRGB vColorError(90,0,10);
  ArduinoOTA.onStart([&] () {
    Periphery().mDisplay.SetText(0, String("Update Start..."));
    Periphery().mDisplay.Run(); // main "loop" is not running
    Periphery().mLEDRing.DisplayProgress(0.0f, vColorProgress, vColorProgressBackground);
  });
  ArduinoOTA.onEnd([&] () {
    Periphery().mDisplay.SetText(0, String("Update Complete!"));
    Periphery().mDisplay.Run(); // main "loop" is not running
    Periphery().mLEDRing.DisplayProgress(1.0f, vColorSuccess, vColorProgressBackground);
  });
  ArduinoOTA.onProgress([&] (unsigned int aProgress, unsigned int aTotal) {
    unsigned int vProgressPercent = aProgress / (aTotal / 100);
    static unsigned int vProgressPercentLast = -1;
    if (vProgressPercent != vProgressPercentLast) {
      vProgressPercentLast = vProgressPercent;
      Periphery().mDisplay.SetText(0, String("Update: ") + vProgressPercent + "%");
      Periphery().mDisplay.Run(); // main "loop" is not running
      Periphery().mLEDRing.DisplayProgress(0.01f * vProgressPercent, vColorProgress, vColorProgressBackground);
    }
  });
  ArduinoOTA.onError([&] (ota_error_t aError) {
    Periphery().mDisplay.SetText(0, String("Update Error!"));
    Periphery().mDisplay.Run(); // main "loop" is not running
    Periphery().mLEDRing.DisplayProgress(1.0f, vColorError, vColorError);
  });
}


void setup()
{
  // serial communication (for debugging)
  #if M_DEBUGGING
  Serial.begin(115200);
  Serial.println();
  ggDebug::SetStream(mDebugStream);
  #endif

  // object debugging output
  GG_DEBUG();

  // start the file system
  mFileSystem->begin();

  // early init display (for debugging- or user-info)
  Periphery().mDisplay.Begin();
  DisplayTimer().Start();

  // connect to wifi
  WiFi.mode(WIFI_STA);
  ConnectWifiManager();
  WiFiMgr().setDebugOutput(M_DEBUGGING);
  WiFiMgr().setConfigPortalTimeout(60); // 1 minute
  WiFiMgr().autoConnect(mHostName.c_str());
  GG_DEBUG_PRINTF("Connected to: %s\n", WiFi.SSID().c_str());
  GG_DEBUG_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
  WiFiConnection().Begin();

  // connect inputs, outputs, socket-events, ...
  ConnectComponents();

  // initialize eeprom handler
  Data(); // creates data object
  ggValueEEProm::Begin();

  // configure and start web-server
  WebServer().Begin();
  GG_DEBUG_PRINTF("Web server started\n");

  // configure and start web-sockets
  WebSockets().Begin();
  GG_DEBUG_PRINTF("Web sockets started\n");

  // start mdns
  MDNS.begin(mHostName.c_str());
  MDNS.addService("http", "tcp", mWebServerPort);
  MDNS.addService("ws", "tcp", mWebSocketsPort);
  GG_DEBUG_PRINTF("MDNS responder started\n");

  // over the air update
  ArduinoOTA.setHostname(mHostName.c_str());
  ArduinoOTA.begin();
  GG_DEBUG_PRINTF("OTA service started\n");

  // make sure all status and debug messages are sent before communication gets
  // interrupted, just in case hardware pins are needed for some different use.
  Serial.flush();

  // setup connected hardware
  Periphery().Begin();

  // start with main lamp state "off" or "on"
  mLampState.mState = Data().mOn ? ggState::eOn : ggState::eOff;
  GG_DEBUG_PRINTF("Lamp Name: %s\n", Data().mName.Get().c_str());
  
  // update periphery depending on eeprom data
  if (mLampState.mState == ggState::eOn) {
    PeripheryLEDCenterSetChannelBrightness();
    Periphery().mLEDRing.SetOn(true);
  }
}


void loop()
{
  DisplayTimer().Run();
  EditTimer().Run();
  Periphery().Run();
  WebServer().Run();
  WebSockets().Run();
  WiFiConnection().Run();
  MDNS.update();
  ArduinoOTA.handle();
  yield();
}
