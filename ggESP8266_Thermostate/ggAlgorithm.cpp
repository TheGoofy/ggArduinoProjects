#include "ggAlgorithm.h"

#include <math.h>


float ggRoundToSD(float aValue, int aSignificantDigits)
{
  // nothing to do, if value is 0
  if (aValue == 0.0f) return aValue;

  // calculate the order of magnitude / position of decimal point (negative if value below 1)
  int vDecimals = static_cast<int>(log10(fabs(aValue)));
  vDecimals = fabs(aValue) < 1.0f ? vDecimals - 1 : vDecimals;

  // compute a factor, which is a power of 10
  float vFactor = pow(10.0f, vDecimals - aSignificantDigits + 1);

  // divide, round, and multiply
  float vRoundOffset = (aValue > 0.0f) ? 0.5f : -0.5f;
  float vResult = static_cast<long long>(aValue / vFactor + vRoundOffset) * vFactor;

  return vResult;
}

