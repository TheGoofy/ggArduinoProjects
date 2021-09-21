#include <ESP8266WebServer.h> // https://github.com/esp8266/Arduino
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets (by Markus Sattler)
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager (by Tzapu)
#include <Adafruit_PWMServoDriver.h> // https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <LiquidCrystalIO.h> // https://www.arduino.cc/en/Reference/LiquidCrystal
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ArduinoJson.h>      // https://github.com/bblanchon/ArduinoJson (by Benoit Blanchon)

#define M_DEBUGGING false
#define M_TEST_ENVIRONMENT false

#define M_VERSION_SW "v1.0.1"

#include "ggWebServer.h"
#include "ggWebSockets.h"
#include "ggWiFiConnection.h"
#include "ggPeriphery.h"
#include "ggAlarmClockNTP.h"
#include "ggLampAlarm.h"
#include "ggData.h"
#include "ggLookupTableT.h"
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


ggAlarmClockNTP& AlarmClock()
{
  static ggAlarmClockNTP* vAlarmClock = nullptr;
  if (vAlarmClock == nullptr) vAlarmClock = new ggAlarmClockNTP("ch.pool.ntp.org", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");
  return *vAlarmClock;
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
    vData = new ggData();
    DataReset();
  }
  return *vData;
}


void DataReset()
{
  ggValueEEProm::cLazyWriter vLazyWriter;
  Data().Name() = mHostName;
  Data().On() = false;
  Data().TransitionTime() = 0.3f;
  Data().SetCurrentSceneIndex(0);
  Data().CurrentScene().mName = "Default Scene";
  for (auto& vBrightness : Data().CurrentScene().mBrightnesses) vBrightness = 0.35f;
  for (auto& vColor : Data().CurrentScene().mColors) vColor = ggColor::cHSV::DarkOrange();
  Data().SetNumScenes(4);
  for (uint16_t vSceneIndex = 0; vSceneIndex < Data().GetNumScenes(); vSceneIndex++) {
    Data().Scene(vSceneIndex).mName = String("Scene ") + (char)('A' + vSceneIndex);
  }
}


void WebSocketsUpdateSceneNames(int aClientID = -1)
{
  GG_DEBUG();
  WebSockets().UpdateSceneNames(Data().Scene(0).mName,
                                Data().Scene(1).mName,
                                Data().Scene(2).mName,
                                Data().Scene(3).mName,
                                aClientID);
}


bool DataLimitTotalBrightness()
{
  GG_DEBUG();
  float vBrightnessTotal = 0.0f;
  for (const ggData::tBrightness& vBrightness : Data().CurrentScene().mBrightnesses) {
    GG_DEBUG_PRINTF("vBrightness = %f\n", vBrightness.Get());
    vBrightnessTotal += vBrightness;
  }
  for (const ggData::tColor& vColor : Data().CurrentScene().mColors) {
    float vColorBrightness = ((uint16_t)vColor.Get().mV * (uint16_t)(255 - (vColor.Get().mS >> 1))) / 65025.0f;
    GG_DEBUG_PRINTF("vColorBrightness = %f\n", vColorBrightness);
    vBrightnessTotal += 0.5f * vColorBrightness;
  }
  GG_DEBUG_PRINTF("vBrightnessTotal = %f\n", vBrightnessTotal);
  const float vBrightnessTotalMax = 3.0f;
  if (vBrightnessTotal > vBrightnessTotalMax) {
    ggValueEEProm::cLazyWriter vLazyWriter;
    const float vScale = vBrightnessTotalMax / vBrightnessTotal;
    for (ggData::tBrightness& vBrightness : Data().CurrentScene().mBrightnesses) {
      vBrightness *= vScale;
    }
    return true;
  }
  return false;
}


void DataSetBrightness(const float& aB0, const float& aB1, const float& aB2, const float& aB3, const float& aB4, const float& aB5)
{
  GG_DEBUG();
  ggValueEEProm::cLazyWriter vLazyWriter;
  Data().CurrentScene().mBrightnesses[0] = aB0;
  Data().CurrentScene().mBrightnesses[1] = aB1;
  Data().CurrentScene().mBrightnesses[2] = aB2;
  Data().CurrentScene().mBrightnesses[3] = aB3;
  Data().CurrentScene().mBrightnesses[4] = aB4;
  Data().CurrentScene().mBrightnesses[5] = aB5;
  DataLimitTotalBrightness();
}


void DataChangeBrightness(const float& aBrightnessLogDelta)
{
  GG_DEBUG();

  static const float vLogBase = 100.0f;
  static ggLookupTableT<float> vLinearToLog(0.0f, 1.0f, [] (const float& aInput) {
    return (log((vLogBase - 1.0f) * aInput + 1.0f) / log(vLogBase));
  });
  static ggLookupTableT<float> vLogToLinear(0.0f, 1.0f, [] (const float& aInput) {
    return (pow(vLogBase, aInput) - 1.0f) / (vLogBase - 1.0f);
  });

  ggValueEEProm::cLazyWriter vLazyWriter;
  for (ggData::tBrightness& vBrightness : Data().CurrentScene().mBrightnesses) {
    vBrightness = vLogToLinear(vLinearToLog(vBrightness) + aBrightnessLogDelta);
  }

  DataLimitTotalBrightness();
}


void PeripheryLEDCenterSetChannelBrightnesses(float aTransitionTime)
{
  GG_DEBUG();
  Periphery().mLEDCenter.ForEachChannel([&aTransitionTime] (int aChannel) {
    Periphery().mLEDCenter.SetChannelBrightness(aChannel, Data().CurrentScene().mBrightnesses[aChannel], aTransitionTime);
  });
}


void WebSocketsUpdateChannelBrightnesses(int aClientID = -1)
{
  GG_DEBUG();
  WebSockets().UpdateChannelBrightness(Data().CurrentScene().mBrightnesses[0],
                                       Data().CurrentScene().mBrightnesses[1],
                                       Data().CurrentScene().mBrightnesses[2],
                                       Data().CurrentScene().mBrightnesses[3],
                                       Data().CurrentScene().mBrightnesses[4],
                                       Data().CurrentScene().mBrightnesses[5],
                                       aClientID);
}


void AdjustTotalBrightness()
{
  if (DataLimitTotalBrightness()) {
    PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime());
    WebSocketsUpdateChannelBrightnesses();
  }
}


void DataSetColors(const ggColor::cHSV& aHSV, ggLocations::tEnum aLocations)
{
  GG_DEBUG();
  ggValueEEProm::cLazyWriter vLazyWriter;
  ggLocations::ForEach([&] (ggLocations::tEnum aLocation) {
    if (ggLocations::Intersect(aLocation, aLocations)) {
      Data().CurrentScene().mColors[ggLocations::ToIndex(aLocation)] = aHSV;
    }
  });
}


void DataChangeColors(int aColorChannel, int aDelta)
{
  ggColor::cHSV vHSV = Data().CurrentScene().mColors[0];
  int vValue = vHSV.mChannels[aColorChannel];
  switch (aColorChannel) {
    case 0: vValue = (vValue + aDelta) & 0xff; break;
    case 1: vValue = ggClamp<int>(vValue - 2 * aDelta, 0, 255); break;
    case 2: vValue = ggClamp<int>(vValue + aDelta, 0, 255); break;
  }
  vHSV.mChannels[aColorChannel] = vValue;
  DataSetColors(vHSV, ggLocations::eAll);
}


void PeripheryLEDRingSetColors(float aTransitionTime)
{
  GG_DEBUG();
  Periphery().mLEDRing.SetColorsT(Data().CurrentScene().mColors, aTransitionTime);
}


void WebSocketsUpdateRingColorHSV(int aClientID = -1)
{
  GG_DEBUG();
  const ggColor::cHSV& vHSV0(Data().CurrentScene().mColors[0]);
  const ggColor::cHSV& vHSV1(Data().CurrentScene().mColors[1]);
  const ggColor::cHSV& vHSV2(Data().CurrentScene().mColors[2]);
  const ggColor::cHSV& vHSV3(Data().CurrentScene().mColors[3]);
  WebSockets().UpdateRingColorHSV(vHSV0.mH, vHSV0.mS, vHSV0.mV,
                                  vHSV1.mH, vHSV1.mS, vHSV1.mV,
                                  vHSV2.mH, vHSV2.mS, vHSV2.mV,
                                  vHSV3.mH, vHSV3.mS, vHSV3.mV,
                                  aClientID);
}


void UpdateDisplay()
{
  GG_DEBUG();
  Periphery().mDisplay.Clear();
  Periphery().mDisplay.SetTitle(Data().Name());
  Periphery().mDisplay.SetText(0, WiFi.localIP().toString());
}


struct ggState {

  enum tEnum {
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
  enum tEnum {
    eClick,
    eClickLong,
    eTimeout
  };
};


class ggLampState {

public:

  ggState::tEnum mState;
  
  typedef std::function<void (ggState::tEnum aState, float aTransitionTime)> tStateFunc;
  tStateFunc mStateFunc;
  
  ggLampState()
  : mState(ggState::eOff),
    mStateFunc(nullptr) {
  }

  ggState::tEnum GetState() const {
    return mState;
  }

  void SetState(ggState::tEnum aState, float aTransitionTime = -1.0) {
    if (mState != aState) {
      mState = aState;
      if (mStateFunc) mStateFunc(mState, aTransitionTime);
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


void AlarmClockSetAlarms()
{
  AlarmClock().Alarms().clear();
  for (uint16_t vIndex = 0; vIndex < Data().GetNumAlarms(); vIndex++) {
    const auto& vDataAlarm = Data().Alarm(vIndex).Get();
    std::shared_ptr<ggLampAlarm> vLampAlarm = std::make_shared<ggLampAlarm>();
    vLampAlarm->mID = vDataAlarm.mID;
    vLampAlarm->mActive = vDataAlarm.mActive;
    vLampAlarm->mMin = vDataAlarm.mMin;
    vLampAlarm->mHour = vDataAlarm.mHour;
    vLampAlarm->mDays = vDataAlarm.mDays;
    vLampAlarm->mSceneIndex = vDataAlarm.mSceneIndex;
    vLampAlarm->mTransitionTime = vDataAlarm.mTransitionTime;
    vLampAlarm->OnAlarm([] (ggAlarmClockNTP::cAlarm& aAlarm) {
      GG_DEBUG_BLOCK("OnAlarm");
      ggLampAlarm* vLampAlarm = static_cast<ggLampAlarm*>(&aAlarm);
      if (vLampAlarm != nullptr) {
        GG_DEBUG_PRINTF("mID = %d\n", vLampAlarm->mID);
        if (vLampAlarm->mDays == ggAlarmClockNTP::eNoDay) { // once
          uint16_t vAlarmIndex = Data().GetAlarmIndex(vLampAlarm->mID);
          if (vAlarmIndex < Data().GetNumAlarms()) {
            ggData::cAlarm vAlarm = Data().Alarm(vAlarmIndex);
            vAlarm.mActive = false;
            Data().Alarm(vAlarmIndex) = vAlarm;
            WebSockets().UpdateAlarmsTable();
          }
        }
        if (vLampAlarm->mSceneIndex == -1) {
          mLampState.SetState(ggState::eOff, vLampAlarm->mTransitionTime);
        }
        else {
          Data().SetCurrentSceneIndex(vLampAlarm->mSceneIndex);
          if (mLampState.mState == ggState::eOn) {
            PeripheryLEDCenterSetChannelBrightnesses(vLampAlarm->mTransitionTime);
            PeripheryLEDRingSetColors(vLampAlarm->mTransitionTime);
          }
          else {
            mLampState.SetState(ggState::eOn, vLampAlarm->mTransitionTime);
          }
          WebSockets().UpdateCurrentSceneIndex(Data().GetCurrentSceneIndex());
          WebSocketsUpdateChannelBrightnesses();
          WebSocketsUpdateRingColorHSV();
        }
      }
    });
    AlarmClock().Alarms().push_back(vLampAlarm);
  }
}


// wifimanager may be connected before any other comonent (in order to indicate AP-mode)
void ConnectWifiManager()
{
  WiFiMgr().setAPCallback([] (WiFiManager* aWiFiManager) {
    Periphery().mDisplay.SetOn(true);
    Periphery().mDisplay.SetTitle(WiFiMgr().getConfigPortalSSID());
    Periphery().mDisplay.SetText(0, "WiFi config...");
    Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
  });
  WiFiMgr().setSaveConfigCallback([] () {
    Periphery().mDisplay.SetText(0, "Config saved!");
    Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
  });
}


void ConnectComponents()
{
  // mode
  mLampState.OnState([&] (ggState::tEnum aState, float aTransitionTime) {
    const float vTransitionTime = (aTransitionTime > 0.0f) ? aTransitionTime : Data().TransitionTime().Get();
    switch (aState) {
      case ggState::eOff:
        Data().On() = false;
        Periphery().mSwitchPSU.SetDelayOff(M_SWITCH_PSU_OFF_DELAY + vTransitionTime);
        Periphery().mSwitchPSU.SetOff(vTransitionTime);
        Periphery().mLEDCenter.SetChannelBrightnesses(0.0f, vTransitionTime);
        Periphery().mLEDRing.SetColorsBlack(vTransitionTime);
        Periphery().mDisplay.SetOn(false);
        WebSockets().UpdateOn(false);
        break;
      case ggState::eOn:
        Data().On() = true;
        Periphery().mSwitchPSU.SetOn(vTransitionTime);
        Periphery().mLEDRing.DisplayNormal();
        // PeripheryLEDCenterSetChannelBrightnesses(vTransitionTime); updated delayed when PSU is on
        // PeripheryLEDRingSetColors(vTransitionTime); colors are updated delayed when PSU is on
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
          Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
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
        PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime());
        WebSocketsUpdateChannelBrightnesses();
        EditTimer().Start();
        break;
      case ggState::eEditColorChannel0:
      case ggState::eEditColorChannel1:
      case ggState::eEditColorChannel2:
        DataChangeColors(ggState::GetColorChannelIndex(mLampState.GetState()), aValueDelta);
        PeripheryLEDRingSetColors(Data().TransitionTime());
        WebSocketsUpdateRingColorHSV();
        AdjustTotalBrightness();
        EditTimer().Start();
        break;
    }
  });

  // PSU switch on is delayed => need to update color ring
  Periphery().mSwitchPSU.OnSwitchedOn([&] (float aTransitionTime) {
    PeripheryLEDCenterSetChannelBrightnesses(aTransitionTime);
    PeripheryLEDRingSetColors(aTransitionTime);
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
    WebSockets().UpdateName(Data().Name(), aClientID);
    WebSockets().UpdateOn(Data().On(), aClientID);
    WebSocketsUpdateSceneNames(aClientID);
    WebSockets().UpdateCurrentSceneIndex(Data().GetCurrentSceneIndex(), aClientID);
    WebSocketsUpdateChannelBrightnesses(aClientID);
    WebSocketsUpdateRingColorHSV(aClientID);
    WebSockets().UpdateTransitionTime(Data().TransitionTime(), aClientID);
  });
  WebSockets().OnClientDisconnect([&] (int aClientID) {
    GG_DEBUG_BLOCK("WebSockets().OnClientDisonnect(...)");
    GG_DEBUG_PRINTF("aClientID = %d\n", aClientID);
  });
  WebSockets().OnSetName([&] (const String& aName) {
    GG_DEBUG_BLOCK("WebSockets().OnSetName(...)");
    GG_DEBUG_PRINTF("aName = %s\n", aName.c_str());
    Data().Name() = aName;
    WebSockets().UpdateName(Data().Name());
    Periphery().mDisplay.SetTitle(Data().Name());
  });
  WebSockets().OnSetOn([&] (bool aOn) {
    mLampState.SetState(aOn ? ggState::eOn : ggState::eOff, Data().TransitionTime());
  });
  WebSockets().OnSetSceneName([&] (uint16_t aIndex, const String& aName) {
    Data().Scene(aIndex).mName = aName;
    WebSockets().UpdateSceneName(aIndex, Data().Scene(aIndex).mName);
  });
  WebSockets().OnSetCurrentSceneIndex([&] (uint16_t aIndex) {
    Data().SetCurrentSceneIndex(aIndex);
    if (mLampState.mState == ggState::eOn) {
      PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime());
      PeripheryLEDRingSetColors(Data().TransitionTime());
    }
    else {
      mLampState.SetState(ggState::eOn, Data().TransitionTime());
    }
    WebSockets().UpdateCurrentSceneIndex(Data().GetCurrentSceneIndex());
    WebSocketsUpdateChannelBrightnesses();
    WebSocketsUpdateRingColorHSV();
  });
  WebSockets().OnSetChannelBrightness([&] (float aB0, float aB1, float aB2, float aB3, float aB4, float aB5) {
    DataSetBrightness(aB0, aB1, aB2, aB3, aB4, aB5);
    PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime());
    WebSocketsUpdateChannelBrightnesses();
  });
  WebSockets().OnSetRingColorHSV([&] (uint8_t aH, uint8_t aS, uint8_t aV, uint8_t aLocations) {
    DataSetColors(ggColor::cHSV(aH, aS, aV), (ggLocations::tEnum)aLocations);
    PeripheryLEDRingSetColors(Data().TransitionTime());
    WebSocketsUpdateRingColorHSV();
    AdjustTotalBrightness();
  });
  WebSockets().OnSetTransitionTime([&] (float aTransitionTime) {
    Data().TransitionTime() = aTransitionTime;
    WebSockets().UpdateTransitionTime(Data().TransitionTime());
  });
  
  // web server
  WebServer().OnAddAlarm([] () -> bool {
    GG_DEBUG_BLOCK("mWebServer.OnAddAlarm(...)");
    bool vAlarmAdded = Data().AddAlarm();
    if (vAlarmAdded) {
      AlarmClockSetAlarms();
      WebSockets().UpdateAlarmsTable();
    }
    return vAlarmAdded;
  });
  WebServer().OnDelAlarm([] (int aAlarmID) -> bool {
    GG_DEBUG_BLOCK("mWebServer.OnDelAlarm(...)");
    bool vAlarmDeleted = Data().RemoveAlarmID(aAlarmID);
    if (vAlarmDeleted) {
      AlarmClockSetAlarms();
      WebSockets().UpdateAlarmsTable();
    }
    return vAlarmDeleted;
  });
  WebServer().OnGetAlarms([] () -> String {
    GG_DEBUG_BLOCK("mWebServer.OnGetAlarms(...)");
    return "{" + Data().AlarmsToJson() + ",\n" + Data().SceneNamesToJson() + "}";
  });
  WebServer().OnSetAlarm([] (const String& aAlarmJson) -> bool {
    GG_DEBUG_BLOCK("mWebServer.OnSetAlarm(...)");
    StaticJsonDocument<200> vAlarmJsonDoc;
    DeserializationError vJsonError = deserializeJson(vAlarmJsonDoc, aAlarmJson);
    GG_DEBUG_PRINTF("vJsonError = %s\n", vJsonError.c_str());
    if (vJsonError) return false;
    uint16_t vAlarmID = vAlarmJsonDoc["mID"].as<uint16_t>();
    uint16_t vAlarmIndex = Data().GetAlarmIndex(vAlarmID);
    if (vAlarmIndex >= Data().GetNumAlarms()) return false;
    ggData::cAlarm vAlarm = Data().Alarm(vAlarmIndex);
    if (vAlarm.mID != vAlarmID) return false;
    vAlarm.mActive = vAlarmJsonDoc["mActive"].as<bool>();
    vAlarm.mMin = vAlarmJsonDoc["mMin"].as<uint8_t>();
    vAlarm.mHour = vAlarmJsonDoc["mHour"].as<uint8_t>();
    vAlarm.mDays = vAlarmJsonDoc["mDays"].as<uint8_t>();
    vAlarm.mSceneIndex = vAlarmJsonDoc["mSceneIndex"].as<int8_t>();
    vAlarm.mTransitionTime = vAlarmJsonDoc["mTransitionTime"].as<float>();
    Data().Alarm(vAlarmIndex) = vAlarm;
    AlarmClockSetAlarms();
    WebSockets().UpdateAlarmsTable();
    return true;
  });
  WebServer().OnGetTime([] () -> String {
    GG_DEBUG_BLOCK("mWebServer.OnGetTime(...)");
    return "{\"mTimeT\":" + String(AlarmClock().GetTimeT()) + "}";
  });
  WebServer().OnPlay([] (const String& aPlayJson) {
    GG_DEBUG_BLOCK("mWebServer.OnPlay(...)");
    GG_DEBUG_PRINTF("aPlayJson = %s\n", aPlayJson.c_str());
    StaticJsonDocument<200> vPlayJsonDoc;
    DeserializationError vJsonError = deserializeJson(vPlayJsonDoc, aPlayJson);
    GG_DEBUG_PRINTF("vJsonError = %s\n", vJsonError.c_str());
    float vPlayTransitionTime = 0.0; // transition-time won't work as long as this animation uses "delay"
    Periphery().mLEDCenter.SetChannelBrightnesses(0.0f, vPlayTransitionTime);
    Periphery().mLEDRing.SetColorsBlack(vPlayTransitionTime);
    uint16_t vDelayMillis = 100;
    for (int vCount = 0; vCount < 5; vCount++) {
      Periphery().mLEDCenter.SetChannelBrightness(5, 0.0, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDCenter.SetChannelBrightness(2, 0.5, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDRing.SetColors(ggColor::cHSV::Black(), ggLocations::eA, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDRing.SetColors(ggColor::cHSV(15,255,255), ggLocations::eB, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDCenter.SetChannelBrightness(2, 0.0, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDCenter.SetChannelBrightness(5, 0.5, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDRing.SetColors(ggColor::cHSV::Black(), ggLocations::eB, vPlayTransitionTime); delay(vDelayMillis);
      Periphery().mLEDRing.SetColors(ggColor::cHSV(15,255,255), ggLocations::eA, vPlayTransitionTime); delay(vDelayMillis);
    }
    PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime());
    PeripheryLEDRingSetColors(Data().TransitionTime());
  });
  WebServer().OnDebugStream([] (Stream& aStream) {
    ggStreams vStreams;
    vStreams.push_back(&aStream);
    vStreams.push_back(&mDebugStream);
    ggDebug::SetStream(vStreams);
    {
      ggDebug vDebug("WebServer().OnDebugStream(...)");
      vDebug.PrintF("Version SW = %s (%s)\n", M_VERSION_SW, __DATE__);
      vDebug.PrintF("Version HW = %s\n", M_VERSION_HW);
      vDebug.PrintF("Environment config = %s\n", M_TEST_ENVIRONMENT ? "TEST" : "NORMAL");
      vDebug.PrintF("mHostName = %s\n", mHostName.c_str());
      vDebug.PrintF("mMillisUpTime = %llu (%.1f Days)\n", mMillisUpTime, mMillisUpTime/(1000.0f*60.0f*60.0f*24.0f));
      Data().PrintDebug("Data()");
      Periphery().PrintDebug("Periphery()");
      DisplayTimer().PrintDebug("DisplayTimer()");
      EditTimer().PrintDebug("EditTimer()");
      AlarmClock().PrintDebug("AlarmClock()");
      WiFiConnection().PrintDebug("WiFiConnection()");
    }
    ggDebug::SetStream(mDebugStream);
  });
  WebServer().OnResetAll([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnResetAll(...)");
      Periphery().mDisplay.SetText(0, String("Reset All..."));
      Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
      DataReset();
      WiFiMgr().resetSettings();
      delay(1000);
    }
    ESP.restart();
  });
  WebServer().OnResetWifi([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnResetWifi(...)");
      Periphery().mDisplay.SetText(0, String("Reset WiFi..."));
      Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
      WiFiMgr().resetSettings();
      delay(1000);
    }
    ESP.restart();
  });
  WebServer().OnReboot([] () {
    {
      GG_DEBUG_BLOCK("mWebServer.OnReboot(...)");
      Periphery().mDisplay.SetText(0, String("Rebooting..."));
      Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
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
    Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
    Periphery().mLEDRing.DisplayProgress(0.0f, vColorProgress, vColorProgressBackground);
  });
  ArduinoOTA.onEnd([&] () {
    Periphery().mDisplay.SetText(0, String("Update Complete!"));
    Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
    Periphery().mLEDRing.DisplayProgress(1.0f, vColorSuccess, vColorProgressBackground);
  });
  ArduinoOTA.onProgress([&] (unsigned int aProgress, unsigned int aTotal) {
    unsigned int vProgressPercent = aProgress / (aTotal / 100);
    static unsigned int vProgressPercentLast = -1;
    if (vProgressPercent != vProgressPercentLast) {
      vProgressPercentLast = vProgressPercent;
      Periphery().mDisplay.SetText(0, String("Update: ") + vProgressPercent + "%");
      Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
      Periphery().mLEDRing.DisplayProgress(0.01f * vProgressPercent, vColorProgress, vColorProgressBackground);
    }
  });
  ArduinoOTA.onError([&] (ota_error_t aError) {
    Periphery().mDisplay.SetText(0, String("Update Error!"));
    Periphery().mDisplay.Run(); // since main "loop" is not running, the display needs to be updated explicitly here
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
  Data(); // creates data object with default values
  ggValueEEProm::Begin(); // loads data values from eeprom
  AlarmClockSetAlarms();

  AlarmClock().Begin();

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
  mLampState.mState = Data().On() ? ggState::eOn : ggState::eOff;
  GG_DEBUG_PRINTF("Lamp Name: %s\n", Data().Name().Get().c_str());
  
  // update periphery depending on eeprom data
  if (mLampState.mState == ggState::eOn) {
    Periphery().mSwitchPSU.SetOn(Data().TransitionTime());
    // PeripheryLEDCenterSetChannelBrightnesses(Data().TransitionTime()); updated delayed when PSU is on
    // PeripheryLEDRingSetColors(Data().TransitionTime()); colors are updated delayed when PSU is on
  }
}


void loop()
{
  AlarmClock().Run();
  DisplayTimer().Run();
  EditTimer().Run();
  Periphery().Run();
  WebServer().Run();
  WebSockets().Run();
  WiFiConnection().Run();
  MDNS.update();
  ArduinoOTA.handle();
  UpdateUpTime();
  yield();
}
