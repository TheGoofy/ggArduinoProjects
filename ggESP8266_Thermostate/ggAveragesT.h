#pragma once

#include <math.h>
#include "ggAlgorithm.h"

/**
 * Computes the averages: Sum, Min, Max, Mean, StdDev, and
 * Variation. The class requires a minimum amount of memory, and
 * does not store individual values (for this reason it can not
 * compute the Median). Also known as "Online-Averages" ...
 *
 * see: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 * 
 * TValue: input number type
 * TValueS: statistics number type
 */
template <class TValue, class TValueS = double>
class ggAveragesT
{

public:

  inline ggAveragesT()
  : mShiftK(0),
    mSum(0),
    mSumOfSquares(0),
    mCount(0),
    mMin(),
    mMax() {
  }

  // reset the averages
  inline void Reset() {
    mShiftK = 0;
    mSum = 0;
    mSumOfSquares = 0;
    mCount = 0;
    mMin = TValue();
    mMax = TValue();
  }

  // adds a sample
  inline void AddValue(TValue aValue,
                       TValueS aCount = 1) {
    const TValueS vValue = ggRound<TValueS>(aValue);
    if (mCount == 0) {
      mShiftK = vValue;
      mMin = aValue;
      mMax = aValue;
    }
    else {
      if (aValue < mMin) mMin = aValue;
      if (aValue > mMax) mMax = aValue;
    }
    const TValueS vValueShifted = vValue - mShiftK;
    mSum += aCount * vValueShifted;
    mSumOfSquares += aCount * vValueShifted * vValueShifted;
    mCount += aCount;
  }

  // add/merge samples from other averages
  inline void AddValues(const ggAveragesT& aOther) {
    if (mCount == 0) {
      *this = aOther;
    }
    else {
      if (aOther.mMin < mMin) mMin = aOther.mMin;
      if (aOther.mMax > mMax) mMax = aOther.mMax;
      TValueS mDeltaShiftK = aOther.mShiftK - mShiftK;
      mSum += aOther.mSum + aOther.mCount * mDeltaShiftK;
      mSumOfSquares += aOther.mSumOfSquares +
                       2 * mDeltaShiftK * (aOther.mSum + aOther.mCount * aOther.mShiftK) +
                       aOther.mCount * (mShiftK * mShiftK - aOther.mShiftK * aOther.mShiftK);
      mCount += aOther.mCount;
    }
  }

  // removes a sample (min and max are not updated)
  inline void RemoveValue(TValue aValue,
                          TValueS aCount = 1) {
    const TValueS vValue = ggRound<TValueS>(aValue);
    const TValueS vValueShifted = vValue - mShiftK;
    mSum -= aCount * vValueShifted;
    mSumOfSquares -= aCount * vValueShifted * vValueShifted;
    mCount -= aCount;
    if (mCount == 0.0) {
      Reset();
    }
  }

  // returns the number of samples
  inline TValueS GetNumberOfSamples() const {
    return mCount;
  }

  // returns the sum of all samples
  inline TValueS GetSum() const {
    return GetSumS();
  }

  // returns the minimum
  inline TValue GetMin() const {
    return mMin;
  }

  // returns the maximum
  inline TValue GetMax() const {
    return mMax;
  }

  // returns the difference between min and max
  inline TValue GetRange() const {
    return mMax - mMin;
  }
  
  // returns the mean value
  inline TValueS GetMean() const {
    if (mCount > 0.0) {
      return GetMeanS();
    }
    return TValue();
  }

  // sum of squared errors, also known as "residual sum of scquares" (RSS, or SSR, or SSE)
  inline TValueS GetSumOfSquaredErrors() const {
    if (mCount > 0.0) {
      return GetSumOfSquaredErrorsS();
    }
    return TValue();
  }

  // variance of sample (square of standard deviation)
  inline TValueS GetVariance() const {
    if (mCount > 1.0) {
      TValueS vSumOfSquaredErrors = GetSumOfSquaredErrorsS();
      return vSumOfSquaredErrors / (mCount - 1.0);
    }
    return TValue();
  }

  // variance of population (square the standard deviation)
  inline TValueS GetVarianceP() const {
    if (mCount > 0.0) {
      TValueS vSumOfSquaredErrors = GetSumOfSquaredErrorsS();
      return vSumOfSquaredErrors / mCount;
    }
    return TValue();
  }

  // returns the standard deviation of sample
  inline TValueS GetStdDev() const {
    if (mCount > 1.0) {
      TValueS vSumOfSquaredErrors = GetSumOfSquaredErrorsS();
      if (vSumOfSquaredErrors >= 0.0) return sqrt(vSumOfSquaredErrors / (mCount - 1));
    }
    return TValue();
  }

  // returns the standard deviation of population
  inline TValueS GetStdDevP() const {
    if (mCount > 0.0) {
      TValueS vSumOfSquaredErrors = GetSumOfSquaredErrorsS();
      if (vSumOfSquaredErrors >= 0.0) return sqrt(vSumOfSquaredErrors / mCount);
    }
    return TValue();
  }

  // returns the coefficient of variation
  inline TValueS GetVariationCoefficient() const {
    if (mCount > 1.0) {
      TValueS vMean = GetMeanS();
      if (vMean != 0.0) {
        TValueS vSumOfSquaredErrors = GetSumOfSquaredErrorsS();
        return sqrt(vSumOfSquaredErrors / (mCount - 1.0)) / vMean;
      }
    }
    return TValue();
  }

private:

  // sum
  inline TValueS GetSumS() const {
    return mSum + mCount * mShiftK;
  }

  // mean, internal (note: mCount must not be zero)
  inline TValueS GetMeanS() const {
    return mSum / mCount + mShiftK;
  }

  // sum of squared errors, internal (note: mCount must not be zero)
  inline TValueS GetSumOfSquaredErrorsS() const {
    return mSumOfSquares - mSum * mSum / mCount;
  }

  TValueS mShiftK;
  TValueS mSum;
  TValueS mSumOfSquares;
  TValueS mCount;
  TValue mMin;
  TValue mMax;

};
