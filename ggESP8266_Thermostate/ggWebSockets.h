#pragma once

#include <WebSocketsServer.h>
#include "ggFunctionParser.h"

class ggWebSockets {

public:

  typedef std::function<void(int aClientNumber)> tClientConnectFunc;
  typedef std::function<void(int aControlMode)> tSetControlModeFunc;
  typedef std::function<void(bool aValue)> tSetBoolValueFunc;
  typedef std::function<void(float aValue)> tSetFloatValueFunc;

  ggWebSockets(int aPort)
  : mServer(aPort),
    mClientConnectFunc(nullptr),
    mSetControlModeFunc(nullptr),
    mSetTemperatureRefFunc(nullptr),
    mSetHysteresisFunc(nullptr),
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

  void UpdateSensorStatus(const char* aSensorStatus, int aClientID = -1) {
    UpdateClientTXT(String("UpdateSensorStatus(\"") + aSensorStatus + "\")", aClientID);
  }

  void UpdateTemperature(float aTemperature, int aClientID = -1) {
    UpdateClientTXT(String("UpdateTemperature(") + aTemperature + ")", aClientID);
  }

  void UpdateHumidity(float aHumidity, int aClientID = -1) {
    UpdateClientTXT(String("UpdateHumidity(") + aHumidity + ")", aClientID);
  }

  void UpdateControlMode(int aControlMode, int aClientID = -1) {
    UpdateClientTXT(String("UpdateControlMode(") + aControlMode + ")", aClientID);
  }

  void UpdateTemperatureRef(float aTemperatureRef, int aClientID = -1) {
    UpdateClientTXT(String("UpdateTemperatureRef(") + aTemperatureRef + ")", aClientID);
  }

  void UpdateHysteresis(float aHysteresis, int aClientID = -1) {
    UpdateClientTXT(String("UpdateHysteresis(") + aHysteresis + ")", aClientID);
  }

  void UpdateOutputAnalog(bool aOutputAnalog, int aClientID = -1) {
    UpdateClientTXT(String("UpdateOutputAnalog(") + aOutputAnalog + ")", aClientID);
  }

  void UpdateKey(bool aKey, int aClientID = -1) {
    UpdateClientTXT(String("UpdateKey(") + aKey + ")", aClientID);
  }

  void UpdateStatusLED(bool aStatusLED, int aClientID = -1) {
    UpdateClientTXT(String("UpdateStatusLED(") + aStatusLED + ")", aClientID);
  }

  void UpdateOutput(float aOutput, int aClientID = -1) {
    UpdateClientTXT(String("UpdateOutput(") + aOutput + ")", aClientID);
  }

  void OnClientConnect(tClientConnectFunc aClientConnectFunc) {
    mClientConnectFunc = aClientConnectFunc;
  }

  void OnSetControlMode(tSetControlModeFunc aSetControlModeFunc) {
    mSetControlModeFunc = aSetControlModeFunc;
  }

  void OnSetTemperatureRef(tSetFloatValueFunc aSetTemperatureRefFunc) {
    mSetTemperatureRefFunc = aSetTemperatureRefFunc;
  }

  void OnSetHysteresis(tSetFloatValueFunc aSetHysteresisFunc) {
    mSetHysteresisFunc = aSetHysteresisFunc;
  }

  void OnSetOutputAnalog(tSetBoolValueFunc aSetOutputAnalogFunc) {
    mSetOutputAnalogFunc = aSetOutputAnalogFunc;
  }

  void OnSetOutput(tSetFloatValueFunc aSetOutputFunc) {
    mSetOutputFunc = aSetOutputFunc;
  }

private:

  inline void UpdateClientTXT(String aTXT, int aClientID = -1) {
    aClientID == -1 ? mServer.broadcastTXT(aTXT) : mServer.sendTXT(aClientID, aTXT);
  }

  void OnEvent(uint8_t aClientID,
               WStype_t aEventType,
               uint8_t* aPayLoad,
               size_t aPayLoadLength) {

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
    if (vFunction.GetName() == "SetControlMode") {
      if (mSetControlModeFunc != nullptr) mSetControlModeFunc(vFunction.ArgInt(0));
      return;
    }
    if (vFunction.GetName() == "SetTemperatureRef") {
      if (mSetTemperatureRefFunc != nullptr) mSetTemperatureRefFunc(vFunction.ArgFloat(0));
      return;
    }
    if (vFunction.GetName() == "SetHysteresis") {
      if (mSetHysteresisFunc != nullptr) mSetHysteresisFunc(vFunction.ArgFloat(0));
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
  tSetControlModeFunc mSetControlModeFunc;
  tSetFloatValueFunc mSetTemperatureRefFunc;
  tSetFloatValueFunc mSetHysteresisFunc;
  tSetBoolValueFunc mSetOutputAnalogFunc;
  tSetFloatValueFunc mSetOutputFunc;
  
};

