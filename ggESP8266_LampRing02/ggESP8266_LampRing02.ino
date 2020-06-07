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
#include "ggValueEEPromString.h"
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


ggValueEEPromString<> mName(mHostName);


void UpdateDisplay()
{
  GG_DEBUG();
  Periphery().mDisplay.Clear();
  Periphery().mDisplay.SetTitle(mName.Get());
  Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
}


void WebSocketsUpdateChannelBrightness(int aClientID = -1)
{
  GG_DEBUG();
  WebSockets().UpdateChannelBrightness(Periphery().mLEDCenter.GetChannelBrightness(0),
                                       Periphery().mLEDCenter.GetChannelBrightness(1),
                                       Periphery().mLEDCenter.GetChannelBrightness(2),
                                       Periphery().mLEDCenter.GetChannelBrightness(3),
                                       Periphery().mLEDCenter.GetChannelBrightness(4),
                                       Periphery().mLEDCenter.GetChannelBrightness(5),
                                       aClientID);
}


void PeripheryLEDCenterSetChannelBrightness(const float& aB0, const float& aB1, const float& aB2, const float& aB3, const float& aB4, const float& aB5)
{
  GG_DEBUG();
  Periphery().mLEDCenter.SetChannelBrightness(0, aB0);
  Periphery().mLEDCenter.SetChannelBrightness(1, aB1);
  Periphery().mLEDCenter.SetChannelBrightness(2, aB2);
  Periphery().mLEDCenter.SetChannelBrightness(3, aB3);
  Periphery().mLEDCenter.SetChannelBrightness(4, aB4);
  Periphery().mLEDCenter.SetChannelBrightness(5, aB5);
}


void WebSocketsUpdateRingColorHSV(int aClientID = -1)
{
  GG_DEBUG();
  WebSockets().UpdateRingColorHSV(Periphery().mLEDRing.GetColorHSV().mH,
                                  Periphery().mLEDRing.GetColorHSV().mS,
                                  Periphery().mLEDRing.GetColorHSV().mV,
                                  aClientID);
}


struct ggState {

  typedef enum tEnum {
    eOff,
    eOn,
    eEditChannel0,
    eEditChannel1,
    eEditChannel2,
    eEditReset,
    eResetWiFi
  };

  static int GetChannelIndex(tEnum aState) {
    switch (aState) {
      case eEditChannel0: return 0;
      case eEditChannel1: return 1;
      case eEditChannel2: return 2;
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
        if (aEvent == ggEvent::eClickLong) SetState(ggState::eEditChannel0);
        break;
      case ggState::eEditChannel0:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditChannel1);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditChannel1:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditChannel2);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditChannel2:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditReset);
        if (aEvent == ggEvent::eTimeout) SetState(ggState::eOn);
        break;
      case ggState::eEditReset:
        if (aEvent == ggEvent::eClick) SetState(ggState::eEditChannel0);
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
        Periphery().SetOff();
        WebSockets().UpdateOn(Periphery().GetOn());
        break;
      case ggState::eOn:
        Periphery().SetOn();
        Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
        Periphery().mLEDRing.DisplayNormal();
        WebSockets().UpdateOn(Periphery().GetOn());
        break;
      case ggState::eEditChannel0:
        Periphery().mDisplay.SetText(0, "Color Hue");
        Periphery().mLEDRing.DisplayChannel(0);
        EditTimer().Reset();
        break;
      case ggState::eEditChannel1:
        Periphery().mDisplay.SetText(0, "Saturaton");
        Periphery().mLEDRing.DisplayChannel(1);
        EditTimer().Reset();
        break;
      case ggState::eEditChannel2:
        Periphery().mDisplay.SetText(0, "Brightness");
        Periphery().mLEDRing.DisplayChannel(2);
        EditTimer().Reset();
        break;
      case ggState::eEditReset:
        Periphery().mDisplay.SetText(0, "Reset");
        Periphery().mLEDRing.DisplayColor(ggColor::cRGB::Orange());
        EditTimer().Reset();
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
  });

  // long press changes mode
  Periphery().mButton.OnPressedFor(2000, [&] () {
    mLampState.HandleEvent(ggEvent::eClickLong);
    vLongPressed = true;
  });

  // rotary encoder signal (4 increments per tick and 20 ticks per revolution)
  Periphery().mEncoder.OnValueChangedDelta([&] (long aValueDelta) {
    switch (mLampState.GetState()) {
      case ggState::eOn:
        Periphery().mLEDCenter.ChangeBrightness(0.25f * 0.05f * aValueDelta);
        WebSocketsUpdateChannelBrightness();
        EditTimer().Reset();
        break;
      case ggState::eEditChannel0:
      case ggState::eEditChannel1:
      case ggState::eEditChannel2:
        Periphery().mLEDRing.ChangeChannel(ggState::GetChannelIndex(mLampState.GetState()), aValueDelta);
        WebSocketsUpdateRingColorHSV();
        EditTimer().Reset();
        break;
    }
  });

  // display
  Periphery().mDisplay.OnConnection([] (bool aConnected) {
    if (aConnected) UpdateDisplay();
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
    WebSockets().UpdateName(mName.Get(), aClientID);
    WebSockets().UpdateOn(Periphery().GetOn(), aClientID);
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
    mName.Set(aName);
    WebSockets().UpdateName(mName.Get());
    Periphery().mDisplay.SetTitle(aName);
  });
  WebSockets().OnSetOn([&] (bool aOn) {
    mLampState.SetState(aOn ? ggState::eOn : ggState::eOff);
  });
  WebSockets().OnSetChannelBrightness([&] (float aB0, float aB1, float aB2, float aB3, float aB4, float aB5) {
    PeripheryLEDCenterSetChannelBrightness(aB0, aB1, aB2, aB3, aB4, aB5);
    WebSockets().UpdateChannelBrightness(aB0, aB1, aB2, aB3, aB4, aB5);
  });
  WebSockets().OnSetRingColorHSV([&] (uint32_t aH, uint32_t aS, uint32_t aV) {
    Periphery().mLEDRing.SetColor(ggColor::cHSV(aH, aS, aV));
    WebSockets().UpdateRingColorHSV(aH, aS, aV);
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
      mName.Set(mHostName);
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

  // connect to wifi
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
  ggValueEEProm::Begin();
  mLampState.mState = Periphery().GetOn() ? ggState::eOn : ggState::eOff;
  GG_DEBUG_PRINTF("Lamp Name: %s\n", mName.Get().c_str());

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
}


void loop()
{
  EditTimer().Run();
  Periphery().Run();
  WebServer().Run();
  WebSockets().Run();
  WiFiConnection().Run();
  MDNS.update();
  ArduinoOTA.handle();
  yield();
}
