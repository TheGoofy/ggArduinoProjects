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
    mCircularFile(aFileName, aDuration / aPeriod, aFileSystem) {
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
    mPressureAVG.AssignValues(vMeasurements.mPressure, 100.0f, -800.0f); // common range: 600..1000hPa (https://de.wikipedia.org/wiki/Luftdruck)
    mTemperatureAVG.AssignValues(vMeasurements.mTemperature, 100.0f, 0.0f); // common range: -20..40°C
    mHumidityAVG.AssignValues(vMeasurements.mHumidity, 100.0f, 0.0f); // common range: 0..100%
    mOutputAVG.AssignValues(vMeasurements.mOutput, 10000.0f, 0.0f); // common range: 0..1
    mCircularFile.Write(aTime, vMeasurements);
  }

private:

  typedef struct cValue {
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
    void AssignValues(cValue& aValue, float aScale = 1.0f, float aOffset = 0.0f) {
      aValue.mMean = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMean()));
      aValue.mMin = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMin()));
      aValue.mMax = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMax()));
      aValue.mStdDev = ggRound<int16_t>(aScale * (aOffset + mAverages.GetStdDev()));
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

};
