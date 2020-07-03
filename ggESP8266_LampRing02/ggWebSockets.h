#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"
#include "ggDebug.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(float, float, float, float, float, float)> tSetChannelBrightnessFunc;
  typedef std::function<void(uint8_t, uint8_t, uint8_t, uint8_t)> tSetRingColorHSVFunc;
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

  void UpdateChannelBrightness(const float& aB0, const float& aB1, const float& aB2, const float& aB3, const float& aB4, const float& aB5, int aClientID = -1) {
    const int vPrecision = 6;
    UpdateClientTXT(String("UpdateChannelBrightness(") + String(aB0, vPrecision) + ","
                                                       + String(aB1, vPrecision) + ","
                                                       + String(aB2, vPrecision) + ","
                                                       + String(aB3, vPrecision) + ","
                                                       + String(aB4, vPrecision) + ","
                                                       + String(aB5, vPrecision) + ")", aClientID);
  }

  void UpdateRingColorHSV(uint8_t aH0, uint8_t aS0, uint8_t aV0,
                          uint8_t aH1, uint8_t aS1, uint8_t aV1,
                          uint8_t aH2, uint8_t aS2, uint8_t aV2,
                          uint8_t aH3, uint8_t aS3, uint8_t aV3,
                          int aClientID = -1) {
    UpdateClientTXT(String("UpdateRingColorHSV(") + aH0 + "," + aS0 + "," + aV0 + ","
                                                  + aH1 + "," + aS1 + "," + aV1 + ","
                                                  + aH2 + "," + aS2 + "," + aV2 + ","
                                                  + aH3 + "," + aS3 + "," + aV3 + ")", aClientID);
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

  void OnSetChannelBrightness(tSetChannelBrightnessFunc aSetChannelBrightnessFunc) {
    mSetChannelBrightnessFunc = aSetChannelBrightnessFunc;
  }

  void OnSetRingColorHSV(tSetRingColorHSVFunc aSetRingColorHSVFunc) {
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
      if (mSetNameFunc != nullptr) {
        mSetNameFunc(vFunction.Arg(0));
      }
      return;
    }
    if (vFunction.GetName() == "SetOn" && vFunction.NumArgs() == 1) {
      if (mSetOnFunc != nullptr) {
        mSetOnFunc(vFunction.ArgBool(0));
      }
      return;
    }
    if (vFunction.GetName() == "SetChannelBrightness") {
      if (mSetChannelBrightnessFunc != nullptr && vFunction.NumArgs() == 6) {
        mSetChannelBrightnessFunc(vFunction.ArgFloat(0),
                                  vFunction.ArgFloat(1),
                                  vFunction.ArgFloat(2),
                                  vFunction.ArgFloat(3),
                                  vFunction.ArgFloat(4),
                                  vFunction.ArgFloat(5));
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
    GG_DEBUG_PRINTF("Unhandled Function!\n");
  }

  WebSocketsServer mServer;

  tClientConnectFunc mClientConnectFunc;
  tClientConnectFunc mClientDisconnectFunc;
  tSetStringValueFunc mSetNameFunc;
  tSetBoolValueFunc mSetOnFunc;
  tSetChannelBrightnessFunc mSetChannelBrightnessFunc;
  tSetRingColorHSVFunc mSetRingColorHSVFunc;
  
};
