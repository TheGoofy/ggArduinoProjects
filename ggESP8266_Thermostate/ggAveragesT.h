#pragma once

#include <math.h>
#include "ggAlgorithm.h"

/**
 * Computes the averages: Sum, Min, Max, Mean, StdDev, and
 * Variation. The class requires a minimum amount of memory, and
 * does not store individual samples (for this reason it can not
 * compute the Median). Also known as "Online-Averages" ...
 *
 * see: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
 */
template <class TValueType>
class ggAveragesT
{

public:

  inline ggAveragesT()
  : mShiftK(0.0),
    mSum(0.0),
    mSumOfSquares(0.0),
    mCount(0.0),
    mMin(),
    mMax() {
  }

  // reset the running averages
  inline void Reset() {
    mShiftK = 0.0;
    mSum = 0.0;
    mSumOfSquares = 0.0;
    mCount = 0.0;
    mMin = TValueType();
    mMax = TValueType();
  }

  // adds a sample
  inline void AddSample(TValueType aValue,
                        double aCount = 1.0) {
    const double vValue = ggRound<double>(aValue);
    if (mCount == 0.0) {
      mShiftK = vValue;
      mMin = aValue;
      mMax = aValue;
    }
    else {
      if (aValue < mMin) mMin = aValue;
      if (aValue > mMax) mMax = aValue;
    }
    const double vValueShifted = vValue - mShiftK;
    mSum += aCount * vValueShifted;
    mSumOfSquares += aCount * vValueShifted * vValueShifted;
    mCount += aCount;
  }

  // removes a sample (min and max are not updated)
  inline void RemoveSample(TValueType aValue,
                           double aCount = 1.0) {
    const double vValue = ggRound<double>(aValue);
    const double vValueShifted = vValue - mShiftK;
    mSum -= aCount * vValueShifted;
    mSumOfSquares -= aCount * vValueShifted * vValueShifted;
    mCount -= aCount;
    if (mCount == 0.0) {
      Reset();
    }
  }

  // returns the number of samples
  inline double GetNumberOfSamples() const {
    return mCount;
  }

  // returns the sum of all samples
  inline double GetSum() const {
    return GetSumDouble();
  }

  // returns the minimum
  inline TValueType GetMin() const {
    return mMin;
  }

  // returns the maximum
  inline TValueType GetMax() const {
    return mMax;
  }

  // returns the mean value
  inline double GetMean() const {
    if (mCount > 0.0) {
      return GetMeanDouble();
    }
    return TValueType();
  }

  // sum of squared errors, also known as "residual sum of scquares" (RSS, or SSR, or SSE)
  inline double GetSumOfSquaredErrors() const {
    if (mCount > 0.0) {
      return GetSumOfSquaredErrorsDouble();
    }
    return TValueType();
  }

  // variance of sample (square of standard deviation)
  inline double GetVariance() const {
    if (mCount > 1.0) {
      double vSumOfSquaredErrors = GetSumOfSquaredErrorsDouble();
      return vSumOfSquaredErrors / (mCount - 1.0);
    }
    return TValueType();
  }

  // variance of population (square the standard deviation)
  inline double GetVarianceP() const {
    if (mCount > 0.0) {
      double vSumOfSquaredErrors = GetSumOfSquaredErrorsDouble();
      return vSumOfSquaredErrors / mCount;
    }
    return TValueType();
  }

  // returns the standard deviation of sample
  inline double GetStdDev() const {
    if (mCount > 1.0) {
      double vSumOfSquaredErrors = GetSumOfSquaredErrorsDouble();
      if (vSumOfSquaredErrors >= 0.0) return sqrt(vSumOfSquaredErrors / (mCount - 1));
    }
    return TValueType();
  }

  // returns the standard deviation of population
  inline double GetStdDevP() const {
    if (mCount > 0.0) {
      double vSumOfSquaredErrors = GetSumOfSquaredErrorsDouble();
      if (vSumOfSquaredErrors >= 0.0) return sqrt(vSumOfSquaredErrors / mCount);
    }
    return TValueType();
  }

  // returns the coefficient of variation
  inline double GetVariationCoefficient() const {
    if (mCount > 1.0) {
      double vMean = GetMeanDouble();
      if (vMean != 0.0) {
        double vSumOfSquaredErrors = GetSumOfSquaredErrorsDouble();
        return sqrt(vSumOfSquaredErrors / (mCount - 1.0)) / vMean;
      }
    }
    return TValueType();
  }

private:

  // sum
  inline double GetSumDouble() const {
    return mSum + mCount * mShiftK;
  }

  // mean, internal (note: mCount must not be zero)
  inline double GetMeanDouble() const {
    return mSum / mCount + mShiftK;
  }

  // sum of squared errors, internal (note: mCount must not be zero)
  inline double GetSumOfSquaredErrorsDouble() const {
    return mSumOfSquares - mSum * mSum / mCount;
  }

  double mShiftK;
  double mSum;
  double mSumOfSquares;
  double mCount;
  TValueType mMin;
  TValueType mMax;

};