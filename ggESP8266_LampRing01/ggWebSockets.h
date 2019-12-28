#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"
#include "ggDebug.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(bool aValue)> tSetBoolValueFunc;
  typedef std::function<void(uint32_t aClientNumber)> tSetUInt32ValueFunc;
  typedef std::function<void(float aValue)> tSetFloatValueFunc;
  typedef std::function<void(const String& aValue)> tSetStringValueFunc;

  ggWebSockets(int aPort)
  : mServer(aPort),
    mClientConnectFunc(nullptr),
    mSetNameFunc(nullptr),
    mSetOnFunc(nullptr),
    mSetCenterBrightnessFunc(nullptr),
    mSetRingColorFunc(nullptr) {
  }

  void Begin() {
    mServer.onEvent([&] (uint8_t aClientID,
                         WStype_t aEventType,
                         uint8_t* aPayLoad,
                         size_t aPayLoadLength) {
      OnEvent(aClientID, aEventType, aPayLoad, aPayLoadLength);
    });
    mServer.begin();
  }

  void Run() {
    mServer.loop();
  }
  
  void UpdateName(const String& aName, int aClientID = -1) {
    UpdateClientTXT(String("UpdateName(\"") + aName + "\")", aClientID);
  }

  void UpdateOn(bool aOn, int aClientID = -1) {
    UpdateClientTXT(String("UpdateOn(") + aOn + ")", aClientID);
  }

  void UpdateCenterBrightness(float aBrightness, int aClientID = -1) {
    UpdateClientTXT(String("UpdateCenterBrightness(") + aBrightness + ")", aClientID);
  }

  void UpdateRingColor(uint32_t aColor, int aClientID = -1) {
    UpdateClientTXT(String("UpdateRingColor('") + RGBToHTML(aColor) + "')", aClientID);
  }

  void OnClientConnect(tClientConnectFunc aClientConnectFunc) {
    mClientConnectFunc = aClientConnectFunc;
  }

  void OnSetName(tSetStringValueFunc aSetNameFunc) {
    mSetNameFunc = aSetNameFunc;
  }

  void OnSetOn(tSetBoolValueFunc aSetOnFunc) {
    mSetOnFunc = aSetOnFunc;
  }

  void OnSetCenterBrightness(tSetFloatValueFunc aSetBrightnessFunc) {
    mSetCenterBrightnessFunc = aSetBrightnessFunc;
  }

  void OnSetRingColor(tSetUInt32ValueFunc aSetRingColorFunc) {
    mSetRingColorFunc = aSetRingColorFunc;
  }

private:

  static ggColor::cRGB HTMLToRGB(const String& aColorHTML) {
    // skip first char "#", convert with base 16 (hex)
    ggColor::cRGB vRGB(strtoul(aColorHTML.c_str() + 1, nullptr, 16));
    // Serial.printf("ggWebSockets::HTMLToRGB(...) - aColorHTML=%s ==> vRGB=%d/%d/%d\n",
    //               aColorHTML.c_str(), vRGB.mR, vRGB.mG, vRGB.mB);
    return vRGB;
  }

  static String RGBToHTML(const ggColor::cRGB& aRGB) {
    char vColorHTML[8]; // 8 chars: 1 prefix, 6 digits, 1 zero terminator (#123456\0)
    sprintf(vColorHTML, "#%06x", aRGB.mData); // 6 hex digits with leading zeroes
    // Serial.printf("ggWebSockets::RGBToHTML(...) - aRGB=%d/%d/%d ==> vColorHTML=%s\n",
    //               aRGB.mR, aRGB.mG, aRGB.mB, vColorHTML);
    return vColorHTML;
  }

  inline void UpdateClientTXT(String aTXT, int aClientID = -1) {
    aClientID == -1 ? mServer.broadcastTXT(aTXT) : mServer.sendTXT(aClientID, aTXT);
  }

  static const char* ToString(WStype_t aEventType) {
    switch (aEventType) {
      case WStype_ERROR:               return "WStype_ERROR";
      case WStype_DISCONNECTED:        return "WStype_DISCONNECTED";
      case WStype_CONNECTED:           return "WStype_CONNECTED";
      case WStype_TEXT:                return "WStype_TEXT";
      case WStype_BIN:                 return "WStype_BIN";
      case WStype_FRAGMENT_TEXT_START: return "WStype_FRAGMENT_TEXT_START";
      case WStype_FRAGMENT_BIN_START:  return "WStype_FRAGMENT_BIN_START";
      case WStype_FRAGMENT:            return "WStype_FRAGMENT";
      case WStype_FRAGMENT_FIN:        return "WStype_FRAGMENT_FIN";
      default:                         return "WStype_t unknown";
    }
  }

  void OnEvent(uint8_t aClientID,
               WStype_t aEventType,
               uint8_t* aPayLoad,
               size_t aPayLoadLength) {
    char vPayLoad[64];
    memset(vPayLoad, 0, sizeof vPayLoad);
    memcpy(vPayLoad, aPayLoad, std::min<size_t>(aPayLoadLength, sizeof vPayLoad - 1));
    switch (aEventType) {
      case WStype_CONNECTED: {
        // const String vURL((char*)aPayLoad);
        if (mClientConnectFunc != nullptr) mClientConnectFunc(aClientID);
        break;
      }
      case WStype_TEXT: {
        Eval((char*)aPayLoad);
        break;
      }
    }
  }

  void Eval(const String& aText) {
    ggFunctionParser vFunction(aText);
    // vFunction.Print(Serial);
    if (vFunction.GetName() == "SetName") {
      if (mSetNameFunc != nullptr) mSetNameFunc(vFunction.Arg(0));
      return;
    }
    if (vFunction.GetName() == "SetOn") {
      if (mSetOnFunc != nullptr) mSetOnFunc(vFunction.ArgBool(0));
      return;
    }
    if (vFunction.GetName() == "SetCenterBrightness") {
      if (mSetCenterBrightnessFunc != nullptr) mSetCenterBrightnessFunc(vFunction.ArgFloat(0));
      return;
    }
    if (vFunction.GetName() == "SetRingColor") {
      if (mSetRingColorFunc != nullptr) mSetRingColorFunc(HTMLToRGB(vFunction.Arg(0)));
      return;
    }
  }

  WebSocketsServer mServer;

  tClientConnectFunc mClientConnectFunc;
  tSetStringValueFunc mSetNameFunc;
  tSetBoolValueFunc mSetOnFunc;
  tSetFloatValueFunc mSetCenterBrightnessFunc;
  tSetUInt32ValueFunc mSetRingColorFunc;
  
};
