#include "ggBase.h"

float ggRoundTo125(float aValue, boolean aRoundDown)
{
  if (aValue == 0.0f) {
    return aValue;
  }
  boolean vPositive = aValue > 0.0f;
  int vDecimals = (int)log10(fabs(aValue));
  vDecimals = fabs(aValue) < 1.0f ? vDecimals - 1 : vDecimals;
  float vErrorMin = fabs(aValue);
  float vResult = aValue;
  for (byte vFactorIndex = 0; vFactorIndex < 8; vFactorIndex++) {
    float vFactor = 0.0f;
    switch (vFactorIndex) {
      case 0: vFactor = -10.0f; break;
      case 1: vFactor =  -5.0f; break;
      case 2: vFactor =  -2.0f; break;
      case 3: vFactor =  -1.0f; break;
      case 4: vFactor =   1.0f; break;
      case 5: vFactor =   2.0f; break;
      case 6: vFactor =   5.0f; break;
      case 7: vFactor =  10.0f; break;
      default: break;
    }
    float vValue = vFactor * pow(10.0f, (float)vDecimals);
    float vError = vErrorMin;
    if (!aRoundDown) vError = fabs(aValue - vValue);
    else vError = vPositive ? (aValue - vValue) : (vValue - aValue);
    if ((vError < vErrorMin) && (vError > 0.0f)) {
      vErrorMin = vError;
      vResult = vValue;
    }
  }
  return vResult;
}
