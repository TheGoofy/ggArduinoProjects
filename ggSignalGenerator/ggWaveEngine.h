#ifndef __GG_WAVE_ENGINE__
#define __GG_WAVE_ENGINE__

#include <Arduino.h>

class ggWaveEngine
{

public:

  static ggWaveEngine& GetInstance(volatile unsigned char* aPortDataRegister,
                                   volatile unsigned char* aPortDataDirectionRegister,
                                   const int aSize,
                                   long aSampleMicroSecondsMin);

  void begin();

  void SetupSquare();
  void SetupSine();
  void SetupSawtoothUp();
  void SetupSawtoothDown();
  void SetupTriangle();
  void SetupConst();
  
  void Start();
  void Stop();  

  void SetFrequency(float aFrequency);
  float GetFrequency() const;
  
private:

  ggWaveEngine();
  virtual ~ggWaveEngine();
  
  static ggWaveEngine mInstance;
  
};

#endif // __GG_WAVE_ENGINE__

