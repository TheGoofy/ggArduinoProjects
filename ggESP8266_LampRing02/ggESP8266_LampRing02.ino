#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "ggPeriphery.h"
#include "ggTimer.h"
#include "ggWiFiConnection.h"
#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggDebug.h"
#include "ggValueEEPromString.h"


const String mHostName = "ESP-Lamp-" + String(ESP.getChipId(), HEX);


ggPeriphery& Periphery()
{
  static ggPeriphery* vPeriphery = nullptr;
  if (vPeriphery == nullptr) vPeriphery = new ggPeriphery;
  return *vPeriphery;
}


ggTimer& Timer()
{
  static ggTimer* vTimer = nullptr;
  if (vTimer == nullptr) vTimer = new ggTimer(10.0f); // timeout in 10 seconds
  return *vTimer;
}


WiFiManager& WiFiMgr()
{
  static WiFiManager* vWifiManager = nullptr;
  if (vWifiManager == nullptr) vWifiManager = new WiFiManager();
  return *vWifiManager;
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
  if (vWebServer == nullptr) vWebServer = new ggWebServer(80); // port 80
  return *vWebServer;
}


ggWebSockets& WebSockets()
{
  static ggWebSockets* vWebSockets = nullptr;
  if (vWebSockets == nullptr) vWebSockets = new ggWebSockets(81); // port 81
  return *vWebSockets;
}


ggValueEEPromString<> mName(mHostName);


void UpdateDisplay()
{
  Periphery().mDisplay.Clear();
  Periphery().mDisplay.SetTitle(mName.Get());
  Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
}


struct ggMode {

  typedef enum tEnum {
    eCenter,
    eRingChannel0,
    eRingChannel1,
    eRingChannel2
  };

  static tEnum Toggle(tEnum aMode) {
    switch (aMode) {
      case eCenter: return eRingChannel0;
      case eRingChannel0: return eRingChannel1;
      case eRingChannel1: return eRingChannel2;
      case eRingChannel2: return eCenter;
      default: return eCenter;
    }
  }

};


// wifimanager may be connected before any other comonent (in order to indicate AP-mode)
void ConnectWifiManager()
{
  WiFiMgr().setAPCallback([] (WiFiManager* aWiFiManager) {
    Periphery().mStatusLED.Begin();
    Periphery().mStatusLED.SetWarning(true);
    Periphery().mDisplay.Begin();
    Periphery().mDisplay.SetTitle(WiFiMgr().getConfigPortalSSID());
    Periphery().mDisplay.SetText(0, "WiFi config...");
    Periphery().mDisplay.Run(); // main "loop" is not running
  });
  WiFiMgr().setSaveConfigCallback([] () {
    Periphery().mDisplay.SetText(0, "Config saved!");
    Periphery().mDisplay.Run(); // main "loop" is not running
    Periphery().mStatusLED.SetWarning(false);
  });
}


void ConnectComponents()
{
  // mode
  static ggMode::tEnum vMode = ggMode::eCenter;
  static bool vIgnoreNextReleasedEvent = false;

  // clicking "on/off"
  Periphery().mButton.OnReleased([&] () {
    Timer().Reset();
    if (!vIgnoreNextReleasedEvent) {
      if (vMode == ggMode::eCenter) {
        Periphery().ToggleOnOff();
        WebSockets().UpdateOn(Periphery().GetOn());
      }
      else {
        vMode = ggMode::eCenter;
        Periphery().mLEDRing.DisplayNormal();
      }
    }
    else {
      vIgnoreNextReleasedEvent = false;
    }
  });

  // long press changes mode
  Periphery().mButton.OnPressedFor(2000, [&] () {
    Timer().Reset();
    vIgnoreNextReleasedEvent = true;
    if (!Periphery().mOn) return;
    vMode = ggMode::Toggle(vMode);
    switch (vMode) {
      case ggMode::eCenter: break;
      case ggMode::eRingChannel0: Periphery().mLEDRing.DisplayChannel(0); break;
      case ggMode::eRingChannel1: Periphery().mLEDRing.DisplayChannel(1); break;
      case ggMode::eRingChannel2: Periphery().mLEDRing.DisplayChannel(2); break;
    }
  });

  // rotary encoder signal
  Periphery().mEncoder.OnValueChangedDelta([&] (long aValueDelta) {
    Timer().Reset();
    if (!Periphery().mOn) return;
    // encoder has 4 increments per tick and 20 ticks per revolution, one revolution is 100%
    if (vMode == ggMode::eCenter) {
      Periphery().mLEDCenter.ChangeBrightness(0.25f * 0.05f * aValueDelta);
      WebSockets().UpdateCenterBrightness(Periphery().mLEDCenter.GetBrightness());
    }
    else {
      switch (vMode) {
        case ggMode::eRingChannel0: Periphery().mLEDRing.ChangeChannel(0, aValueDelta); break;
        case ggMode::eRingChannel1: Periphery().mLEDRing.ChangeChannel(1, aValueDelta); break;
        case ggMode::eRingChannel2: Periphery().mLEDRing.ChangeChannel(2, aValueDelta); break;
      }
      WebSockets().UpdateRingColorHSV(Periphery().mLEDRing.GetColorHSV().mH,
                                      Periphery().mLEDRing.GetColorHSV().mS,
                                      Periphery().mLEDRing.GetColorHSV().mV);
    }
  });

  // display
  Periphery().mDisplay.OnConnection([] (bool aConnected) {
    if (aConnected) UpdateDisplay();
  });

  // switch back to normal after a while of no user inputs
  Timer().OnTimeOut([&] () {
    if (vMode != ggMode::eCenter) {
      vMode = ggMode::eCenter;
      Periphery().mLEDRing.DisplayNormal();
    }
  });

  // wifi events
  WiFiConnection().OnConnect([&] () {
    ggDebug vDebug("WiFiConnection().OnConnect()");
    Periphery().mStatusLED.SetWarning(false);
    UpdateDisplay();
  });
  WiFiConnection().OnDisconnect([&] () {
    ggDebug vDebug("WiFiConnection().OnDisconnect()");
    Periphery().mStatusLED.SetWarning(true);
    UpdateDisplay();
  });

  // events from websocket clients ...
  WebSockets().OnClientConnect([&] (int aClientID) { // need to update all sockets
    ggDebug vDebug("WebSockets().OnClientConnect(...)");
    vDebug.PrintF("aClientID = %d\n", aClientID);
    WebSockets().UpdateName(mName.Get(), aClientID);
    WebSockets().UpdateOn(Periphery().GetOn(), aClientID);
    WebSockets().UpdateCenterBrightness(Periphery().mLEDCenter.GetBrightness(), aClientID);
    WebSockets().UpdateRingColorHSV(Periphery().mLEDRing.GetColorHSV().mH,
                                    Periphery().mLEDRing.GetColorHSV().mS,
                                    Periphery().mLEDRing.GetColorHSV().mV,
                                    aClientID);
  });
  WebSockets().OnClientDisconnect([&] (int aClientID) {
    ggDebug vDebug("WebSockets().OnClientDisonnect(...)");
    vDebug.PrintF("aClientID = %d\n", aClientID);
  });
  WebSockets().OnSetName([&] (const String& aName) {
    mName.Set(aName);
    WebSockets().UpdateName(mName.Get());
    Periphery().mDisplay.SetTitle(aName);
  });
  WebSockets().OnSetOn([&] (bool aOn) {
    Periphery().SetOn(aOn);
    WebSockets().UpdateOn(Periphery().GetOn());
  });
  WebSockets().OnSetCenterBrightness([&] (float aBrightness) {
    Periphery().mLEDCenter.SetBrightness(aBrightness);
    WebSockets().UpdateCenterBrightness(aBrightness);
  });
  WebSockets().OnSetRingColorHSV([&] (uint32_t aH, uint32_t aS, uint32_t aV) {
    Periphery().mLEDRing.SetColor(ggColor::cHSV(aH, aS, aV));
    WebSockets().UpdateRingColorHSV(aH, aS, aV);
  });

  // OTA status display
  static ggColor::cRGB vColorProgress(200,0,150);
  static ggColor::cRGB vColorProgressBackground(0,0,0);
  static ggColor::cRGB vColorSuccess(0,200,0);
  static ggColor::cRGB vColorError(255,0,0);
  ArduinoOTA.onStart([&] () {
    Periphery().mLEDCenter.SetOn(false);
    Periphery().mLEDRing.DisplayProgress(0.0f, vColorProgress, vColorProgressBackground);
  });
  ArduinoOTA.onEnd([&] () {
    Periphery().mLEDRing.DisplayProgress(1.0f, vColorSuccess, vColorProgressBackground);
  });
  ArduinoOTA.onProgress([&] (unsigned int aProgress, unsigned int aTotal) {
    float vProgress = static_cast<float>(aProgress) / static_cast<float>(aTotal);
    Periphery().mLEDRing.DisplayProgress(vProgress, vColorProgress, vColorProgressBackground);
  });
  ArduinoOTA.onError([&] (ota_error_t aError) {
    Periphery().mLEDRing.DisplayProgress(1.0f, vColorError, vColorError);
  });
}


void setup()
{
  // setup serial communication (for debugging)
  Serial.begin(115200);
  Serial.println();
  GG_DEBUG();

  // connect to wifi
  ConnectWifiManager();
  WiFiMgr().setDebugOutput(false);
  WiFiMgr().setConfigPortalTimeout(60); // seconds
  WiFiMgr().autoConnect(mHostName.c_str());
  vDebug.PrintF("Connected to: %s\n", WiFi.SSID().c_str());
  vDebug.PrintF("IP address: %s\n", WiFi.localIP().toString().c_str());
  WiFiConnection().Begin();

  // connect inputs, outputs, socket-events, ...
  ConnectComponents();

  // startup eeprom utility class
  ggValueEEProm::Begin();
  vDebug.PrintF("Lamp Name: %s\n", mName.Get().c_str());

  // configure and start web-server
  WebServer().Begin();
  vDebug.PrintF("Web server started\n");

  // configure and start web-sockets
  WebSockets().Begin();
  vDebug.PrintF("Web sockets started\n");
  
  // start mdns
  MDNS.begin(mHostName.c_str());
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  vDebug.PrintF("MDNS responder started\n");

  // over the air update
  ArduinoOTA.setHostname(mHostName.c_str());
  ArduinoOTA.begin();
  vDebug.PrintF("OTA service started\n");

  // make sure all status and debug messages are sent before communication gets
  // interrupted, just in case hardware pins are needed for some different use.
  Serial.flush();
  
  // initialize connected hardware
  Periphery().Begin();
}


void loop()
{
  Periphery().Run();
  WebServer().Run();
  WebSockets().Run();
  WiFiConnection().Run();
  ArduinoOTA.handle();
  MDNS.update();
  yield();
}
