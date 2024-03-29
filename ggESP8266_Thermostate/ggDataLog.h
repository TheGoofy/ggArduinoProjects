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
    mLastSamples(nullptr) {
    GG_DEBUG();
    GG_DEBUG_PRINTF("aPeriod = %d\n", aPeriod);
    GG_DEBUG_PRINTF("aDuration = %d\n", aDuration);
    GG_DEBUG_PRINTF("number of data blocks = %d\n", mCircularFile.GetNumberOfDataBlocks());
    GG_DEBUG_PRINTF("aFileName = \"%s\"\n", aFileName.c_str());
  }

  uint32_t GetPeriod() const {
    return mPeriod;
  }

  const String& GetFileName() const {
    return mCircularFile.GetFileName();
  }

  void ResetAll() {
    mCircularFile.Reset();
    mPressureAVG.mAverages.Reset();
    mTemperatureAVG.mAverages.Reset();
    mHumidityAVG.mAverages.Reset();
    mOutputAVG.mAverages.Reset();
  }

  void AddPressureValue(float aPressure) {
    mPressureAVG.AddValue(aPressure);
  }

  void AddTemperatureSample(float aTemperature) {
    mTemperatureAVG.AddValue(aTemperature);
  }

  void AddHumidityValue(float aHumidity) {
    mHumidityAVG.AddValue(aHumidity);
  }

  void AddOutputValue(float aOutput) {
    mOutputAVG.AddValue(aOutput);
  }

  void AddValues(const ggDataLog& aDataLogSrc) {
    mPressureAVG.AddValues(aDataLogSrc.mPressureAVG);
    mTemperatureAVG.AddValues(aDataLogSrc.mTemperatureAVG);
    mHumidityAVG.AddValues(aDataLogSrc.mHumidityAVG);
    mOutputAVG.AddValues(aDataLogSrc.mOutputAVG);
  }

  void ResetAveragesOnNextAddValue() {
    mPressureAVG.ResetOnNextAddValue();
    mTemperatureAVG.ResetOnNextAddValue();
    mHumidityAVG.ResetOnNextAddValue();
    mOutputAVG.ResetOnNextAddValue();
  }

  void Write(time_t aTime) {
    // BME280 ranges: 300..1100hPa, -40..85°C, 0..100%
    // https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
    cSamples vMeasurements;
    // common range: 600..1000hPa (full uint16 range: 480..1120hPa, 0.01hPa, see: https://de.wikipedia.org/wiki/Luftdruck)
    mPressureAVG.AssignSample(vMeasurements.mPressure, -800.0f, 100.0f, 1000.0f, LastSamples().mPressure);
    // common range: -20..40°C (full uint16 range: -320..320°C, resolution: 0.01°C)
    mTemperatureAVG.AssignSample(vMeasurements.mTemperature, 0.0f, 100.0f, 1000.0f, LastSamples().mTemperature);
    // range: 0..100% (full uint16 range: -320..320%, resolution: 0.01%)
    mHumidityAVG.AssignSample(vMeasurements.mHumidity, 0.0f, 100.0f, 1000.0f, LastSamples().mHumidity);
    // range: 0..1 (full uint16 range: -320..320%, resolution: 0.01%)
    mOutputAVG.AssignSample(vMeasurements.mOutput, 0.0f, 10000.0f, 100000.0f, LastSamples().mOutput);
    // write "time" and "measurements" into circular file
    bool vSuccess = mCircularFile.Write(static_cast<uint32_t>(aTime), vMeasurements);
    if (!vSuccess) {
      GG_DEBUG();
      GG_DEBUG_PRINTF("aTime = %d\n", aTime);
      GG_DEBUG_PRINTF("writing \"%s\"\n", GetFileName().c_str());
      GG_DEBUG_PRINTF("failed to write file ... :-(\n");
    }
  }

private:

  struct cSample {
    cSample() : mMean(0), mMin(0), mMax(0), mStdDev(0) {}
    int16_t mMean;
    int16_t mMin;
    int16_t mMax;
    int16_t mStdDev;
  };

  struct cSamples {
    cSample mPressure;
    cSample mTemperature;
    cSample mHumidity;
    cSample mOutput;
  };

  // helper sub-class for averages
  class cAverages {
  public:
    friend class ggDataLog;
    cAverages()
    : mAverages(),
      mResetOnNextAddValue(false) {
    }
    void AddValue(float aValue) {
      ResetIfNeeded();
      mAverages.AddValue(aValue);
    }
    void AddValues(const cAverages& aAveragesSrc) {
      ResetIfNeeded();
      mAverages.AddValues(aAveragesSrc.mAverages);
    }
    void ResetOnNextAddValue() {
      mResetOnNextAddValue = true;
    }
    void AssignSample(cSample& aSample, float aOffset, float aScale, float aScaleStdDev, cSample& aSampleLast) {
      if (mAverages.GetNumberOfSamples() > 0) {
        aSample.mMean = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMean()));
        aSample.mMin = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMin()));
        aSample.mMax = ggRound<int16_t>(aScale * (aOffset + mAverages.GetMax()));
        aSample.mStdDev = ggRound<int16_t>(aScaleStdDev * mAverages.GetStdDev());
        aSampleLast = aSample;
      }
      else {
        aSample = aSampleLast;
      }
    }
  private:
    void ResetIfNeeded() {
      if (mResetOnNextAddValue) {
        mAverages.Reset();
        mResetOnNextAddValue = false;
      }
    }
    ggAveragesT<float, float> mAverages;
    bool mResetOnNextAddValue;
  };

  static_assert(sizeof(uint32_t) == 4, "Size needs to be binary compatible on client-side! See 'ggLogFile.html'");
  static_assert(sizeof(cSamples) == 4 * 4 * 2, "Size needs to be binary compatible on client-side! See 'ggLogFile.html'");

  typedef ggCircularFileT<uint32_t, cSamples> tCircularFile;

  cAverages mPressureAVG;
  cAverages mTemperatureAVG;
  cAverages mHumidityAVG;
  cAverages mOutputAVG;

  uint32_t mPeriod;
  tCircularFile mCircularFile;

  mutable uint32_t mLastTime;
  mutable cSamples* mLastSamples;
  cSamples& LastSamples() const {
    if (mLastSamples == nullptr) {
      mLastSamples = new cSamples();
      bool vSuccess = mCircularFile.Read(mLastTime, *mLastSamples);
      if (!vSuccess) {
        GG_DEBUG();
        GG_DEBUG_PRINTF("reading \"%s\"\n", GetFileName().c_str());
        GG_DEBUG_PRINTF("failed to read file ... :-(\n");
      }
    }
    return *mLastSamples;
  }

};
