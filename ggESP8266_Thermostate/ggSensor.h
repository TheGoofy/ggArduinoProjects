#pragma once

#include <Wire.h>

#include "ggSampler.h"
#include "ggAlgorithm.h"
#include "ggDebug.h"

class ggSensor {

public:

  typedef std::function<void(float)> tFloatValueChangedFunc;
  typedef std::function<void(const char* aStatus)> tStatusChangedFunc;
  
  ggSensor(int aPinSDA,
           int aPinSCL,
           tFloatValueChangedFunc aPressureChanged = nullptr,
           tFloatValueChangedFunc aTemperatureChanged = nullptr,
           tFloatValueChangedFunc aHumidityChanged = nullptr)
  : mPinSDA(aPinSDA),
    mPinSCL(aPinSCL),
    mBME(),
    mSampler(0.5f),
    mPressure(NAN),
    mTemperature(NAN),
    mHumidity(NAN),
    mStatus(eStatusSensorOK),
    mPressureChanged(aPressureChanged),
    mTemperatureChanged(aTemperatureChanged),
    mHumidityChanged(aHumidityChanged),
    mStatusChangedFunc(nullptr) {   
  }

  void Begin() {
    Wire.begin(mPinSDA, mPinSCL);
    mSampler.OnSample([&] () {
      OnSample();
    });
    Init();
  }

  void OnPressureChanged(tFloatValueChangedFunc aPressureChanged) {
    mPressureChanged = aPressureChanged;
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
    return mStatus == eStatusSensorOK;
  }

  float GetPressure() const {
    return mPressure;
  }

  float GetTemperature() const {
    return mTemperature;
  }

  float GetHumidity() const {
    return mHumidity;
  }

  const char* GetStatus() const {
    switch (mStatus) {
      case eStatusSensorOK: return "Status OK";
      case eStatusSensorNotFound: return "Not found";
      case eStatusSensorModelUnknown: return "Model unknown";
      case eStatusSensorReadFailed: return "Read failed";
      default: return "Status unknown";
    }
  }

  void Run() {
    mSampler.Run();
  }

  void PrintDebug(const String& aName = "") const {
    ggDebug vDebug("ggSensor", aName);
    mSampler.PrintDebug("mSampler");
    vDebug.PrintF("mPinSDA = %d\n", mPinSDA);
    vDebug.PrintF("mPinSCL = %d\n", mPinSCL);
    vDebug.PrintF("mStatus = %s\n", GetStatus());
    vDebug.PrintF("mPressure = %f\n", mPressure);
    vDebug.PrintF("mTemperature = %f\n", mTemperature);
    vDebug.PrintF("mHumidity = %f\n", mHumidity);
  }

private:

  enum tStatus {
    eStatusSensorOK,
    eStatusSensorNotFound,
    eStatusSensorModelUnknown,
    eStatusSensorReadFailed  
  };

  void UpdateStatus(tStatus aStatus) {
    if (mStatus != aStatus) {
      mStatus = aStatus;
      if (mStatusChangedFunc != nullptr) {
        mStatusChangedFunc(GetStatus());
      }
    }
  }

  void Init() {
    // GG_DEBUG();
    if (!mBME.begin()) {
      UpdateStatus(eStatusSensorNotFound);
    }
    else if (mBME.chipModel() == BME280::ChipModel_UNKNOWN) {
      UpdateStatus(eStatusSensorModelUnknown);
    }
    else {
      UpdateStatus(eStatusSensorOK);
    }
  }

  template <typename TValue>
  inline static bool IsFinite(const TValue& aValue) {
    return !isnan(aValue) && !isinf(aValue);
  }

  void OnSample() {

    // (re)init in case sensor has a problem
    if (mStatus != eStatusSensorOK) {
      Init();
    }

    // check if there is no problem
    if (mStatus == eStatusSensorOK) {

      // read the values
      float vPressure(NAN), vTemperature(NAN), vHumidity(NAN);
      mBME.read(vPressure, vTemperature, vHumidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);

      // check read error
      if (!IsFinite(vPressure) || !IsFinite(vTemperature) || !IsFinite(vHumidity)) {
        UpdateStatus(eStatusSensorReadFailed);
      }

      // init members (initially the are "NAN")
      if (!IsFinite(mPressure)) mPressure = vPressure;
      if (!IsFinite(mTemperature)) mTemperature = vTemperature;
      if (!IsFinite(mHumidity)) mHumidity = vHumidity;

      // smooth values with iif filter
      if (IsFinite(vPressure)) vPressure = 0.8f*mPressure + 0.2f*vPressure;
      if (IsFinite(vTemperature)) vTemperature = 0.5f*mTemperature + 0.5f*vTemperature;
      if (IsFinite(vHumidity)) vHumidity = 0.6f*mHumidity + 0.4f*vHumidity;

      /*
      // round to most significant digits
      vPressure = ggRoundToSD(vPressure, 4);
      vTemperature = ggRoundToSD(vTemperature, 4);
      vHumidity = ggRoundToSD(vHumidity, 3);
      */

      // update pressure, if changed
      if (vPressure != mPressure) {
        mPressure = vPressure;
        if (mPressureChanged != nullptr) {
          mPressureChanged(mPressure);
        }
      }

      // update temperature, if changed
      if (vTemperature != mTemperature) {
        mTemperature = vTemperature;
        if (mTemperatureChanged != nullptr) {
          mTemperatureChanged(mTemperature);
        }
      }

      // update humidity, if changed
      if (vHumidity != mHumidity) {
        mHumidity = vHumidity;
        if (mHumidityChanged != nullptr) {
          mHumidityChanged(mHumidity);
        }
      }
    }
  }

  int mPinSDA;
  int mPinSCL;
  BME280I2C mBME;

  ggSampler mSampler;

  tStatus mStatus;  
  float mPressure;
  float mTemperature;
  float mHumidity;

  tStatusChangedFunc mStatusChangedFunc;
  tFloatValueChangedFunc mPressureChanged;
  tFloatValueChangedFunc mTemperatureChanged;
  tFloatValueChangedFunc mHumidityChanged;

};
