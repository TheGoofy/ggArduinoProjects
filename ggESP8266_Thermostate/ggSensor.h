#pragma once

#include "ggSampler.h"

class ggSensor {

public:

  typedef std::function<void(float)> tFloatValueChangedFunc;
  typedef std::function<void(const char* aStatus)> tStatusChangedFunc;
  
  ggSensor(int aPin,
           tFloatValueChangedFunc aTemperatureChanged = nullptr,
           tFloatValueChangedFunc aHumidityChanged = nullptr)
  : mPin(aPin),
    mDHT(),
    mSampler(0.5f),
    mTemperature(0.0f),
    mHumidity(0.0f),
    mStatus(DHTesp::ERROR_NONE),
    mTemperatureChanged(aTemperatureChanged),
    mHumidityChanged(aHumidityChanged),
    mStatusChangedFunc(nullptr) {   
  }

  void Begin() {
    mDHT.setup(mPin, DHTesp::AM2302);
    mSampler.OnSample([&] () {
      OnSample();
    });
  }

  void OnTemperatureChanged(tFloatValueChangedFunc aTemperatureChanged) {
    mTemperatureChanged = aTemperatureChanged;
  }

  void OnHumidityChanged(tFloatValueChangedFunc aHumidityChanged) {
    mHumidityChanged = aHumidityChanged;
  }

  void OnStatusChanged(tStatusChangedFunc aStatusChangedFunc) {
    mStatusChangedFunc = aStatusChangedFunc;
  }

  bool StatusOK() const {
    return mStatus == DHTesp::ERROR_NONE;
  }

  float GetTemperature() const {
    return mTemperature;
  }

  float GetHumidity() const {
    return mHumidity;
  }

  const char* GetStatus() const {
    return const_cast<DHTesp&>(mDHT).getStatusString();
  }

  void Run() {
    mSampler.Run();
  }

private:

  void OnSample() {

    // get the new temperature
    TempAndHumidity vDataDHT = mDHT.getTempAndHumidity();
    
    // check status ...
    DHTesp::DHT_ERROR_t vStatus = mDHT.getStatus();
    if (vStatus != mStatus) {
      mStatus = vStatus;
      if (mStatusChangedFunc != nullptr) {
        mStatusChangedFunc(mDHT.getStatusString());
      }
    }

    // update temerature and humidity, if all OK
    if (vStatus == DHTesp::ERROR_NONE) {
      if (vDataDHT.temperature != mTemperature) {
        mTemperature = vDataDHT.temperature;
        if (mTemperatureChanged != nullptr) {
          mTemperatureChanged(mTemperature);
        }
      }
      if (vDataDHT.humidity != mHumidity) {
        mHumidity = vDataDHT.humidity;
        if (mHumidityChanged != nullptr) {
          mHumidityChanged(mHumidity);
        }
      }
    }
  }

  int mPin;
  DHTesp mDHT;

  ggSampler mSampler;

  DHTesp::DHT_ERROR_t mStatus;
  float mTemperature;
  float mHumidity;

  tStatusChangedFunc mStatusChangedFunc;
  tFloatValueChangedFunc mTemperatureChanged;
  tFloatValueChangedFunc mHumidityChanged;

};

