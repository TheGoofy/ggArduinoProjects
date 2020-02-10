#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"
#include "ggDebug.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(int aControlMode)> tSetControlModeFunc;
  typedef std::function<void(bool aValue)> tSetBoolValueFunc;
  typedef std::function<void(float aValue)> tSetFloatValueFunc;
  typedef std::function<void(float, float, float)> tSetFloatValue3Func;
  typedef std::function<void(const String& aValue)> tSetStringValueFunc;

  ggWebSockets(int aPort)
  : mServer(aPort),
    mClientConnectFunc(nullptr),
    mClientDisconnectFunc(nullptr),
    mSetNameFunc(nullptr),
    mSetControlModeFunc(nullptr),
    mSetTemperatureSetPointFunc(nullptr),
    mSetHysteresisFunc(nullptr),
    mSetControlPIDFunc(nullptr),
    mSetOutputAnalogFunc(nullptr),
    mSetOutputFunc(nullptr) {
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

  void UpdateSensorStatus(const char* aSensorStatus, int aClientID = -1) {
    UpdateClientTXT(String("UpdateSensorStatus(\"") + aSensorStatus + "\")", aClientID);
  }

  void UpdatePressure(float aPressure, int aClientID = -1) {
    UpdateClientTXT(String("UpdatePressure(") + String(aPressure, 2) + ")", aClientID);
  }

  void UpdateTemperature(float aTemperature, int aClientID = -1) {
    UpdateClientTXT(String("UpdateTemperature(") + String(aTemperature, 3) + ")", aClientID);
  }

  void UpdateHumidity(float aHumidity, int aClientID = -1) {
    UpdateClientTXT(String("UpdateHumidity(") + String(aHumidity, 3) + ")", aClientID);
  }

  void UpdateControlMode(int aControlMode, int aClientID = -1) {
    UpdateClientTXT(String("UpdateControlMode(") + aControlMode + ")", aClientID);
  }

  void UpdateTemperatureSetPoint(float aTemperatureSetPoint, int aClientID = -1) {
    UpdateClientTXT(String("UpdateTemperatureSetPoint(") + aTemperatureSetPoint + ")", aClientID);
  }

  void UpdateHysteresis(float aHysteresis, int aClientID = -1) {
    UpdateClientTXT(String("UpdateHysteresis(") + aHysteresis + ")", aClientID);
  }

  void UpdateControlPID(float aP, float aI, float aD, int aClientID = -1) {
    UpdateClientTXT(String("UpdateControlPID(") + String(aP, 3) + "," + String(aI, 3) + "," + String(aD, 3) + ")", aClientID);
  }

  void UpdateOutputAnalog(bool aOutputAnalog, int aClientID = -1) {
    UpdateClientTXT(String("UpdateOutputAnalog(") + aOutputAnalog + ")", aClientID);
  }

  void UpdateKey(bool aKey, int aClientID = -1) {
    UpdateClientTXT(String("UpdateKey(") + aKey + ")", aClientID);
  }

  void UpdateOutput(float aOutput, int aClientID = -1) {
    UpdateClientTXT(String("UpdateOutput(") + aOutput + ")", aClientID);
  }

  void UpdateProgress(const String& aName, uint32_t aValue, uint32_t aValueMax, bool aShow, int aClientID = -1) {
    UpdateClientTXT(String("UpdateProgress(\"") + aName + "\", " + aValue + ", " + aValueMax + ", " + aShow + ")", aClientID);
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

  void OnSetControlMode(tSetControlModeFunc aSetControlModeFunc) {
    mSetControlModeFunc = aSetControlModeFunc;
  }

  void OnSetTemperatureSetPoint(tSetFloatValueFunc aSetTemperatureSetPointFunc) {
    mSetTemperatureSetPointFunc = aSetTemperatureSetPointFunc;
  }

  void OnSetHysteresis(tSetFloatValueFunc aSetHysteresisFunc) {
    mSetHysteresisFunc = aSetHysteresisFunc;
  }

  void OnSetControlPID(tSetFloatValue3Func aSetControlPIDFunc) {
    mSetControlPIDFunc = aSetControlPIDFunc;
  }

  void OnSetOutputAnalog(tSetBoolValueFunc aSetOutputAnalogFunc) {
    mSetOutputAnalogFunc = aSetOutputAnalogFunc;
  }

  void OnSetOutput(tSetFloatValueFunc aSetOutputFunc) {
    mSetOutputFunc = aSetOutputFunc;
  }

  String GetStatus() const {
    return String(const_cast<ggWebSockets&>(*this).mServer.connectedClients()) + "/" + String(WEBSOCKETS_SERVER_CLIENT_MAX);
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
    // vFunction.Print(Serial);
    if (vFunction.GetName() == "SetName") {
      if (mSetNameFunc != nullptr) mSetNameFunc(vFunction.Arg(0));
      return;
    }
    if (vFunction.GetName() == "SetControlMode") {
      if (mSetControlModeFunc != nullptr) mSetControlModeFunc(vFunction.ArgInt(0));
      return;
    }
    if (vFunction.GetName() == "SetTemperatureSetPoint") {
      if (mSetTemperatureSetPointFunc != nullptr) mSetTemperatureSetPointFunc(vFunction.ArgFloat(0));
      return;
    }
    if (vFunction.GetName() == "SetHysteresis") {
      if (mSetHysteresisFunc != nullptr) mSetHysteresisFunc(vFunction.ArgFloat(0));
      return;
    }
    if (vFunction.GetName() == "SetControlPID") {
      if (mSetControlPIDFunc != nullptr) mSetControlPIDFunc(vFunction.ArgFloat(0), vFunction.ArgFloat(1), vFunction.ArgFloat(2));
      return;
    }
    if (vFunction.GetName() == "SetOutputAnalog") {
      if (mSetOutputAnalogFunc != nullptr) mSetOutputAnalogFunc(vFunction.ArgBool(0));
      return;
    }
    if (vFunction.GetName() == "SetOutput") {
      if (mSetOutputFunc != nullptr) mSetOutputFunc(vFunction.ArgFloat(0));
      return;
    }
  }

  WebSocketsServer mServer;

  tClientConnectFunc mClientConnectFunc;
  tClientConnectFunc mClientDisconnectFunc;
  tSetStringValueFunc mSetNameFunc;
  tSetControlModeFunc mSetControlModeFunc;
  tSetFloatValueFunc mSetTemperatureSetPointFunc;
  tSetFloatValueFunc mSetHysteresisFunc;
  tSetFloatValue3Func mSetControlPIDFunc;
  tSetBoolValueFunc mSetOutputAnalogFunc;
  tSetFloatValueFunc mSetOutputFunc;
  
};
