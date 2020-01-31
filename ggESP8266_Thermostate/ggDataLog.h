#pragma once

#include "ggAveragesT.h"
#include "ggCircularFileT.h"

class ggDataLog {
public:

  ggDataLog(uint32_t aPeriod, // time ins seconds from sample to sample
            uint32_t aDuration, // overall recording time in seconds until circular file rolls over
            const String& aFileName, // filename for the circular file (ring-buffer)
            FS* aFileSystem) // file-system ro use
  : mPressureAVG(),
    mTemperatureAVG(),
    mHumidityAVG(),
    mOutputAVG(),
    mPeriod(aPeriod),
    mCircularFile(aFileName, aDuration / aPeriod, aFileSystem),
    mLastTime(0),
    mLastMeasurements(nullptr) {
    GG_DEBUG();
    vDebug.PrintF("aPeriod = %d\n", aPeriod);
    vDebug.PrintF("aDuration = %d\n", aDuration);
    vDebug.PrintF("number of data blocks = %d\n", mCircularFile.GetNumberOfDataBlocks());
    vDebug.PrintF("aFileName = \"%s\"\n", aFileName.c_str());
  }

  uint32_t GetPeriod() const {
    return mPeriod;
  }

  const String& GetFileName() const {
    return mCircularFile.GetFileName();
  }

  void AddPressureSample(float aPressure) {
    mPressureAVG.AddSample(aPressure);
  }

  void AddTemperatureSample(float aTemperature) {
    mTemperatureAVG.AddSample(aTemperature);
  }

  void AddHumiditySample(float aHumidity) {
    mHumidityAVG.AddSample(aHumidity);
  }

  void AddOutputSample(float aOutput) {
    mOutputAVG.AddSample(aOutput);
  }

  void AddSamples(const ggDataLog& aDataLogSrc) {
    mPressureAVG.AddSamples(aDataLogSrc.mPressureAVG);
    mTemperatureAVG.AddSamples(aDataLogSrc.mTemperatureAVG);
    mHumidityAVG.AddSamples(aDataLogSrc.mHumidityAVG);
    mOutputAVG.AddSamples(aDataLogSrc.mOutputAVG);
  }

  void ResetOnNextAddSample() {
    mPressureAVG.ResetOnNextAddSample();
    mTemperatureAVG.ResetOnNextAddSample();
    mHumidityAVG.ResetOnNextAddSample();
    mOutputAVG.ResetOnNextAddSample();
  }

  void Write(time_t aTime) {
    // BME280 ranges: 300..1100hPa, -40..85°C, 0..100%
    // https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
    cMeasurements vMeasurements;
    // common range: 600..1000hPa (full uint16 range: 480..1120hPa, 0.01hPa, see: https://de.wikipedia.org/wiki/Luftdruck)
    mPressureAVG.AssignValues(vMeasurements.mPressure, -800.0f, 100.0f, LastMeasurements().mPressure);
    // common range: -20..40°C (full uint16 range: -320..320°C, resolution: 0.01°C)
    mTemperatureAVG.AssignValues(vMeasurements.mTemperature, 0.0f, 100.0f, LastMeasurements().mTemperature);
    // range: 0..100% (full uint16 range: -320..320%, resolution: 0.01%)
    mHumidityAVG.AssignValues(vMeasurements.mHumidity, 0.0f, 100.0f, LastMeasurements().mHumidity);
    // range: 0..1 (full uint16 range: -320..320%, resolution: 0.01%)
    mOutputAVG.AssignValues(vMeasurements.mOutput, 0.0f, 10000.0f, LastMeasurements().mOutput);
    // write "time" and "measurements" into circular file
    mCircularFile.Write(aTime, vMeasurements);
  }

private:

  typedef struct cValue {
    cValue() : mMean(0), mMin(0), mMax(0), mStdDev(0) {}
    int16_t mMean;
    int16_t mMin;
    int16_t mMax;
    int16_t mStdDev;
  };

  typedef struct cMeasurements {
    cValue mPressure;
    cValue mTemperature;
    cValue mHumidity;
    cValue mOutput;
  };

  // helper sub-class for averages
  class cAverages {
  public:
    typedef ggAveragesT<float, float> tBaseClass;
    cAverages()
    : mAverages(),
      mResetOnNextAddSample(false) {
    }
    void AddSample(float aValue) {
      ResetIfNeeded();
      mAverages.AddSample(aValue);
    }
    void AddSamples(const cAverages& aAveragesSrc) {
      ResetIfNeeded();
      mAverages.AddSamples(aAveragesSrc.mAverages);
    }
    void ResetOnNextAddSample() {
      mResetOnNextAddSample = true;
    }
    void AssignValues(cValue& aValue, float aOffset, float aScale, const cValue& aValueDefault) {
      if (mAverages.GetNumberOfSamples() > 0) {
        aValue.mMean = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMean()));
        aValue.mMin = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMin()));
        aValue.mMax = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMax()));
        aValue.mStdDev = ggRound<int16_t>(aScale * (aOffset + mAverages.GetStdDev()));
      }
      else {
        aValue = aValueDefault;
      }
    }
  private:
    void ResetIfNeeded() {
      if (mResetOnNextAddSample) {
        mAverages.Reset();
        mResetOnNextAddSample = false;
      }
    }
    ggAveragesT<float, float> mAverages;
    bool mResetOnNextAddSample;
  };

  typedef ggCircularFileT<time_t, cMeasurements> tCircularFile;

  cAverages mPressureAVG;
  cAverages mTemperatureAVG;
  cAverages mHumidityAVG;
  cAverages mOutputAVG;

  uint32_t mPeriod;
  tCircularFile mCircularFile;

  mutable time_t mLastTime;
  mutable cMeasurements* mLastMeasurements;
  const cMeasurements& LastMeasurements() const {
    if (mLastMeasurements == nullptr) {
      mLastMeasurements = new cMeasurements();
      mCircularFile.Read(mLastTime, *mLastMeasurements);
    }
    return *mLastMeasurements;
  }

};
