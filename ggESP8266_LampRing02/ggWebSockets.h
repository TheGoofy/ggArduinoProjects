#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"
#include "ggDebug.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(float aValue)> tSetBrightnessFunc;
  typedef std::function<void(int aChannel, float aBrightness)> tSetChannelBrightnessFunc;
  typedef std::function<void(bool aValue)> tSetBoolValueFunc;
  typedef std::function<void(uint32_t aClientNumber)> tSetUInt32ValueFunc;
  typedef std::function<void(uint32_t, uint32_t, uint32_t)> tSet3UInt32ValueFunc;
  typedef std::function<void(const String& aValue)> tSetStringValueFunc;

  ggWebSockets(int aPort)
  : mServer(aPort),
    mClientConnectFunc(nullptr),
    mClientDisconnectFunc(nullptr),
    mSetNameFunc(nullptr),
    mSetOnFunc(nullptr),
    mSetBrightnessFunc(nullptr),
    mSetChannelBrightnessFunc(nullptr),
    mSetRingColorHSVFunc(nullptr) {
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

  void UpdateBrightness(float aBrightness, int aClientID = -1) {
    UpdateClientTXT(String("UpdateBrightness(") + aBrightness + ")", aClientID);
  }

  void UpdateChannelBrightness(int aChannel, float aBrightness, int aClientID = -1) {
    UpdateClientTXT(String("UpdateChannelBrightness(") + aChannel + "," + aBrightness + ")", aClientID);
  }

  void UpdateRingColorHSV(uint8_t aH, uint8_t aS, uint8_t aV, int aClientID = -1) {
    UpdateClientTXT(String("UpdateRingColorHSV(") + aH + "," + aS + "," + aV + ")", aClientID);
  }

  void OnClientConnect(tClientConnectFunc aClientConnectFunc) {
    mClientConnectFunc = aClientConnectFunc;
  }

  void OnClientDisconnect(tClientConnectFunc aClientDisconnectFunc) {
    mClientDisconnectFunc = aClientDisconnectFunc;
  }

  void OnSetName(tSetStringValueFunc aSetNameFunc) {
    mSetNameFunc = aSetNameFunc;
  }

  void OnSetOn(tSetBoolValueFunc aSetOnFunc) {
    mSetOnFunc = aSetOnFunc;
  }

  void OnSetBrightness(tSetBrightnessFunc aSetBrightnessFunc) {
    mSetBrightnessFunc = aSetBrightnessFunc;
  }

  void OnSetChannelBrightness(tSetChannelBrightnessFunc aSetChannelBrightnessFunc) {
    mSetChannelBrightnessFunc = aSetChannelBrightnessFunc;
  }

  void OnSetRingColorHSV(tSet3UInt32ValueFunc aSetRingColorHSVFunc) {
    mSetRingColorHSVFunc = aSetRingColorHSVFunc;
  }

private:

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
    switch (aEventType) {
      case WStype_CONNECTED: {
        GG_DEBUG();
        vDebug.PrintF("aEventType = %s\n", ToString(aEventType));
        vDebug.PrintF("remote IP = %s\n", mServer.remoteIP(aClientID).toString().c_str());
        vDebug.PrintF("aPayLoad = %s\n", (char*)aPayLoad);
        if (mClientConnectFunc != nullptr) mClientConnectFunc(aClientID);
        break;
      }
      case WStype_DISCONNECTED: {
        GG_DEBUG();
        vDebug.PrintF("aEventType = %s\n", ToString(aEventType));
        vDebug.PrintF("remote IP = %s\n", mServer.remoteIP(aClientID).toString().c_str());
        // vDebug.PrintF("aPayLoad = %s\n", (char*)aPayLoad);
        if (mClientDisconnectFunc != nullptr) mClientDisconnectFunc(aClientID);
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
    vFunction.Print(ggDebug::GetStream());
    if (vFunction.GetName() == "SetName" && vFunction.NumArgs() == 1) {
      if (mSetNameFunc != nullptr) mSetNameFunc(vFunction.Arg(0));
      return;
    }
    if (vFunction.GetName() == "SetOn" && vFunction.NumArgs() == 1) {
      if (mSetOnFunc != nullptr) mSetOnFunc(vFunction.ArgBool(0));
      return;
    }
    if (vFunction.GetName() == "SetBrightness") {
      if (mSetBrightnessFunc != nullptr && vFunction.NumArgs() == 1) mSetBrightnessFunc(vFunction.ArgFloat(0));
      return;
    }
    if (vFunction.GetName() == "SetChannelBrightness") {
      if (mSetChannelBrightnessFunc != nullptr && vFunction.NumArgs() == 2) mSetChannelBrightnessFunc(vFunction.ArgInt(0), vFunction.ArgFloat(1));
      return;
    }
    if (vFunction.GetName() == "SetRingColorHSV" && vFunction.NumArgs() == 3) {
      if (mSetRingColorHSVFunc != nullptr) mSetRingColorHSVFunc(vFunction.ArgInt(0), vFunction.ArgInt(1), vFunction.ArgInt(2));
      return;
    }
  }

  WebSocketsServer mServer;

  tClientConnectFunc mClientConnectFunc;
  tClientConnectFunc mClientDisconnectFunc;
  tSetStringValueFunc mSetNameFunc;
  tSetBoolValueFunc mSetOnFunc;
  tSetBrightnessFunc mSetBrightnessFunc;
  tSetChannelBrightnessFunc mSetChannelBrightnessFunc;
  tSet3UInt32ValueFunc mSetRingColorHSVFunc;
  
};
