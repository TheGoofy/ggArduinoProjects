#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"
#include "ggDebug.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(uint16_t, const String&)> tSetSceneNameFunc;
  typedef std::function<void(float)> tSetChannelBrightnessFunc;
  typedef std::function<void(uint8_t, uint8_t, uint8_t, uint8_t)> tSetRingColorHSVFunc;
  typedef std::function<void(bool)> tSetBoolFunc;
  typedef std::function<void(uint16_t)> tSetUInt16Func;
  typedef std::function<void(uint32_t)> tSetUInt32Func;
  typedef std::function<void(uint32_t, uint32_t, uint32_t)> tSet3UInt32Func;
  typedef std::function<void(float)> tSetFloatFunc;
  typedef std::function<void(const String&)> tSetStringFunc;

  ggWebSockets(int aPort)
  : mServer(aPort),
    mClientConnectFunc(nullptr),
    mClientDisconnectFunc(nullptr),
    mSetNameFunc(nullptr),
    mSetOnFunc(nullptr),
    mSetSceneNameFunc(nullptr),
    mSetCurrentSceneIndexFunc(nullptr),
    mSetChannelBrightnessFunc(nullptr),
    mSetRingColorHSVFunc(nullptr),
    mSetTransitionTimeFunc(nullptr) {
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

  void UpdateSceneName(uint16_t aIndex, const String& aName, int aClientID = -1) {
    UpdateClientTXT(String("UpdateSceneName(") + aIndex + ",\"" + aName + "\")", aClientID);
  }

  void UpdateSceneNames(const String& aName0, const String& aName1, const String& aName2, const String& aName3, int aClientID = -1) {
    UpdateClientTXT(String("UpdateSceneNames(\"") + aName0 + "\","
                                           + "\"" + aName1 + "\","
                                           + "\"" + aName2 + "\","
                                           + "\"" + aName3 + "\")", aClientID);
  }

  void UpdateCurrentSceneIndex(uint16_t aIndex, int aClientID = -1) {
    UpdateClientTXT(String("UpdateCurrentSceneIndex(") + aIndex + ")", aClientID);
  }

  void UpdateChannelBrightness(const float& aB0, int aClientID = -1) {
    const int vPrecision = 6;
    UpdateClientTXT(String("UpdateChannelBrightness(") + String(aB0, vPrecision) + ")", aClientID);
  }

  void UpdateRingColorHSV(uint8_t aH0, uint8_t aS0, uint8_t aV0,
                          uint8_t aH1, uint8_t aS1, uint8_t aV1,
                          int aClientID = -1) {
    UpdateClientTXT(String("UpdateRingColorHSV(") + aH0 + "," + aS0 + "," + aV0 + ","
                                                  + aH1 + "," + aS1 + "," + aV1 + ")", aClientID);
  }

  void UpdateTransitionTime(float aTransitionTime, int aClientID = -1) {
    UpdateClientTXT(String("UpdateTransitionTime(") + aTransitionTime + ")", aClientID);
  }

  void UpdateAlarmsTable(int aClientID = -1) {
    UpdateClientTXT(String("UpdateAlarmsTable()"), aClientID);
  }

  void OnClientConnect(tClientConnectFunc aClientConnectFunc) {
    mClientConnectFunc = aClientConnectFunc;
  }

  void OnClientDisconnect(tClientConnectFunc aClientDisconnectFunc) {
    mClientDisconnectFunc = aClientDisconnectFunc;
  }

  void OnSetName(tSetStringFunc aSetNameFunc) {
    mSetNameFunc = aSetNameFunc;
  }

  void OnSetOn(tSetBoolFunc aSetOnFunc) {
    mSetOnFunc = aSetOnFunc;
  }

  void OnSetSceneName(tSetSceneNameFunc aSetSceneNameFunc) {
    mSetSceneNameFunc = aSetSceneNameFunc;
  }

  void OnSetCurrentSceneIndex(tSetUInt16Func aSetCurrentSceneIndexFunc) {
    mSetCurrentSceneIndexFunc = aSetCurrentSceneIndexFunc;
  }

  void OnSetChannelBrightness(tSetChannelBrightnessFunc aSetChannelBrightnessFunc) {
    mSetChannelBrightnessFunc = aSetChannelBrightnessFunc;
  }

  void OnSetRingColorHSV(tSetRingColorHSVFunc aSetRingColorHSVFunc) {
    mSetRingColorHSVFunc = aSetRingColorHSVFunc;
  }

  void OnSetTransitionTime(tSetFloatFunc aSetTransitionTimeFunc) {
    mSetTransitionTimeFunc = aSetTransitionTimeFunc;
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
        GG_DEBUG_PRINTF("aEventType = %s\n", ToString(aEventType));
        GG_DEBUG_PRINTF("remote IP = %s\n", mServer.remoteIP(aClientID).toString().c_str());
        // GG_DEBUG_PRINTF("aPayLoad = %s\n", (char*)aPayLoad);
        if (mClientConnectFunc != nullptr) mClientConnectFunc(aClientID);
        break;
      }
      case WStype_DISCONNECTED: {
        GG_DEBUG();
        GG_DEBUG_PRINTF("aEventType = %s\n", ToString(aEventType));
        GG_DEBUG_PRINTF("remote IP = %s\n", mServer.remoteIP(aClientID).toString().c_str());
        // GG_DEBUG_PRINTF("aPayLoad = %s\n", (char*)aPayLoad);
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
    GG_DEBUG();
    ggFunctionParser vFunction(aText);
    GG_DEBUG_EXECUTE(vFunction.PrintDebug());
    if (vFunction.GetName() == "SetName" && vFunction.NumArgs() == 1) {
      if (mSetNameFunc != nullptr) mSetNameFunc(vFunction.Arg(0));
      return;
    }
    if (vFunction.GetName() == "SetOn" && vFunction.NumArgs() == 1) {
      if (mSetOnFunc != nullptr) mSetOnFunc(vFunction.ArgBool(0));
      return;
    }
    if (vFunction.GetName() == "SetSceneName" && vFunction.NumArgs() == 2) {
      if (mSetSceneNameFunc != nullptr) mSetSceneNameFunc(vFunction.ArgInt(0), vFunction.Arg(1));
      return;
    }
    if (vFunction.GetName() == "SetCurrentSceneIndex" && vFunction.NumArgs() == 1) {
      if (mSetCurrentSceneIndexFunc != nullptr) mSetCurrentSceneIndexFunc(vFunction.ArgInt(0));
      return;
    }
    if (vFunction.GetName() == "SetChannelBrightness" && vFunction.NumArgs() == 1) {
      if (mSetChannelBrightnessFunc != nullptr) {
        mSetChannelBrightnessFunc(vFunction.ArgFloat(0));
      }
      return;
    }
    if (vFunction.GetName() == "SetRingColorHSV" && vFunction.NumArgs() == 4) {
      if (mSetRingColorHSVFunc != nullptr) {
        mSetRingColorHSVFunc(vFunction.ArgInt(0),
                             vFunction.ArgInt(1),
                             vFunction.ArgInt(2),
                             vFunction.ArgInt(3));
      }
      return;
    }
    if (vFunction.GetName() == "SetTransitionTime" && vFunction.NumArgs() == 1) {
      if (mSetTransitionTimeFunc != nullptr) mSetTransitionTimeFunc(vFunction.ArgFloat(0));
      return;
    }
    GG_DEBUG_PRINTF("Unhandled Function!\n");
  }

  WebSocketsServer mServer;

  tClientConnectFunc mClientConnectFunc;
  tClientConnectFunc mClientDisconnectFunc;
  tSetStringFunc mSetNameFunc;
  tSetBoolFunc mSetOnFunc;
  tSetSceneNameFunc mSetSceneNameFunc;
  tSetUInt16Func mSetCurrentSceneIndexFunc;
  tSetChannelBrightnessFunc mSetChannelBrightnessFunc;
  tSetRingColorHSVFunc mSetRingColorHSVFunc;
  tSetFloatFunc mSetTransitionTimeFunc;
  
};
