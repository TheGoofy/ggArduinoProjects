#ifndef __GG_JOY_STICK__
#define __GG_JOY_STICK__

#include <Arduino.h>

class ggJoyStick
{

public:
  
  ggJoyStick(int aPinX, int aPinY, int aPinSwitch);
  
  void begin();
  
  void CalibrateX(int aCenter = 512, int aMin = 0, int aMax = 1023);
  void CalibrateY(int aCenter = 512, int aMin = 0, int aMax = 1023);
  void CalibrateSwitch(boolean aInvert);
  
  float GetX() const;
  float GetY() const;
  boolean GetSwitch() const;
  
private:
  
  float GetValue(int aAxis) const;
  void Calibrate(int aAxis, int aCenter, int aMin, int aMax);
  
  // xy configuration
  int mPin[2];
  int mCenter[2];
  float mScaleL[2];
  float mScaleR[2];
  
  // switch configuration
  int mPinSwitch;
  boolean mSwitchInvert;
  
};

#endif // __GG_JOY_STICK__
