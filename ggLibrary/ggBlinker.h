#ifndef _GG_BLINKER_H__
#define _GG_BLINKER_H__

#include <Arduino.h>

class ggBlinker
{

public:
  
  ggBlinker(byte aPin,
            boolean aInverted = false,
            long aMillisOn = 70,
            long aMillisOff = 130);

  void begin();
  void run();

  void Blink(long aCount = 1);
  void BlinkStart();
  void BlinkStop();
  void BlinkSeconds(float aSeconds);

  bool IsBlinking() const;
  
  void SetMillisOn(long aMillisOn);
  void SetMillisOff(long aMillisOff);
  long GetMillisOn() const;
  long GetMillisOff() const;
  
private:
  
  const byte mPin;
  const boolean mInverted;
  long mMillisStop;
  long mMillisOn;
  long mMillisOff;
  long mCount;
  
};

#endif // _GG_BLINKER_H__
